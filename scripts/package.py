#!/usr/bin/env python3
"""Package verified native executables and corresponding-source references."""
import hashlib
import json
import os
import shutil
import subprocess
from pathlib import Path

root = Path(__file__).resolve().parents[1]
version = '0.1.0-preview.4'
name = f'HoriSpotConnect-{version}-macos-arm64'
build = Path(os.environ.get('HORI_NATIVE_BUILD', root / '.hori-build/native'))
package = root / 'dist' / name

def run(*args, cwd=root):
    return subprocess.check_output(args, cwd=cwd, text=True).strip()

# A package is produced only after this exact build's test suite passes.
subprocess.run(['ctest', '--test-dir', str(build), '--output-on-failure', '--no-tests=error'], check=True)
if package.exists():
    backup = package.with_name(name + '.previous')
    if backup.exists():
        shutil.rmtree(backup)  # Generated package directory, never private runtime state.
    package.rename(backup)
(package / 'bin').mkdir(parents=True)
sources = {
    'HoriSpotConnect': build / 'bridge/spotraop-macos-arm64-static',
    'hori-audio-probe': build / 'audio/hori-audio-probe',
    'hori-discover': build / 'hori-discover',
    'hori-raop-probe': build / 'hori-raop-probe',
    'hori-airplay': root / '.hori-build/airplay-cli-v0.5.8/hori-airplay',
}
for target, source in sources.items():
    dest = package / 'bin' / target
    shutil.copy2(source, dest)
    dest.chmod(0o755)
    description = run('file', str(dest))
    if 'arm64' not in description or 'Mach-O' not in description:
        raise RuntimeError(f'not a native arm64 executable: {target}')
    subprocess.run(['codesign', '--force', '--sign', '-', str(dest)], check=True)
    subprocess.run(['codesign', '--verify', '--strict', str(dest)], check=True)

for item in ('scripts', 'config', 'docs'):
    shutil.copytree(root / item, package / item, ignore=shutil.ignore_patterns('__pycache__'))
for item in ('README.md', 'CHANGELOG.md', 'LICENSE'):
    shutil.copy2(root / item, package / item)

manifest = {'version': version, 'status': 'Preview / Experimental',
            'repository': 'https://github.com/hhhoratioxu/SpotConnect',
            'branch': 'feature/spotify-lossless', 'commit': run('git', 'rev-parse', 'HEAD'),
            'spotify_flac_access': 'UNAVAILABLE', 'submodules': []}
for line in run('git', 'submodule', 'status', '--recursive').splitlines():
    sha, path, *_ = line[1:].split()
    initialized = line[0] != '-'
    if initialized:
        url = run('git', 'remote', 'get-url', 'origin', cwd=root / path)
    else:
        # Nested submodules belong to their immediate superproject's mapping.
        # Their names need not equal their paths or the root repository's keys.
        url = None
        for parent in (root / path).parents:
            mapping = parent / '.gitmodules'
            if not mapping.is_file():
                continue
            relative = str((root / path).relative_to(parent))
            entries = run('git', 'config', '--file', str(mapping), '--get-regexp',
                          r'^submodule\..*\.path$')
            for entry in entries.splitlines():
                key, value = entry.split(None, 1)
                if value == relative:
                    url = run('git', 'config', '--file', str(mapping), '--get',
                              key[:-5] + '.url')
                    break
            if url:
                break
            if parent == root:
                break
        if not url:
            raise RuntimeError(f'missing source URL for nested submodule: {path}')
    manifest['submodules'].append({'path': path, 'commit': sha, 'url': url,
                                   'initialized_for_macos_build': initialized})
manifest['libFLAC'] = {'version': '1.5.0',
    'source': 'https://github.com/xiph/flac/releases/download/1.5.0/flac-1.5.0.tar.xz',
    'sha256': 'f2c1c76592a82ffff8413ba3c4a1299b6c7ab06c734dee03fd88630485c2b920'}
manifest['nativeAirPlaySender'] = {
    'version': '0.5.8', 'license': 'GPL-3.0',
    'repository': 'https://github.com/music-assistant/airplay-cli',
    'source_commit': '1701bc25e9d303b42532569f624b64c0d0afb1f6',
    'original_binary_sha256': '6d460ad08562844d978f337bee28a79b0eea390c84c239b5d66f55624e27d719',
    'source_instructions': 'git clone --recursive --branch v0.5.8 https://github.com/music-assistant/airplay-cli.git; make STATIC=1',
    'transport': 'native AirPlay 2 / NTP / 44100 Hz / 16-bit stereo ALAC',
    'spotify_flac': False}
(package / 'SOURCE-MANIFEST.json').write_text(json.dumps(manifest, indent=2) + '\n')

licenses = package / 'licenses'
licenses.mkdir()
tracked_files = subprocess.check_output(
    ['git', 'ls-files', '--recurse-submodules', '-z'], cwd=root).decode().split('\0')
for relative in tracked_files:
    path = root / relative
    # Only corresponding-source license files may enter the public package.
    # Ignored runtime state and prior test extractions are never source inputs.
    if not relative or not path.is_file():
        continue
    if path.name.upper().startswith(('LICENSE', 'COPYING', 'COPYRIGHT', 'NOTICE')):
        license_name = '__'.join(path.relative_to(root).parts)
        if len(license_name.encode()) > 220:
            license_name = license_name[:160] + '__' + hashlib.sha256(license_name.encode()).hexdigest()[:16]
        shutil.copy2(path, licenses / license_name)
flac_license = root / '.hori-build/flac-1.5.0/COPYING.Xiph'
if not flac_license.exists():
    flac_license = Path(os.environ.get('HORI_FLAC_SOURCE', '')) / 'COPYING.Xiph'
if not flac_license.exists():
    raise RuntimeError('libFLAC license source is missing')
shutil.copy2(flac_license, licenses / 'libFLAC-1.5.0-COPYING.Xiph')
for name in ('LICENSE', 'THIRD_PARTY_NOTICES.md'):
    shutil.copy2(root / 'third_party/airplay-cli' / name, licenses / ('airplay-cli-' + name))
test_xml = build / 'test-results.xml'
if test_xml.exists():
    shutil.copy2(test_xml, package / 'docs/LOCAL-TEST-RESULTS.xml')

checksums = []
for path in sorted(package.rglob('*')):
    if path.is_file():
        checksums.append(f'{hashlib.sha256(path.read_bytes()).hexdigest()}  {path.relative_to(package)}')
(package / 'SHA256SUMS').write_text('\n'.join(checksums) + '\n')
zip_path = root / 'dist' / (name + '.zip')
if zip_path.exists():
    zip_path.rename(zip_path.with_suffix('.zip.previous'))
subprocess.run(['ditto', '-c', '-k', '--sequesterRsrc', '--keepParent', str(package), str(zip_path)], check=True)
digest = hashlib.sha256(zip_path.read_bytes()).hexdigest()
(root / 'dist' / (name + '.zip.sha256')).write_text(f'{digest}  {zip_path.name}\n')
print(f'Packaged Preview: {zip_path}\nSHA256: {digest}')
