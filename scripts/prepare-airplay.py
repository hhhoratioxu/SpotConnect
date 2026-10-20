#!/usr/bin/env python3
"""Fetch the unchanged official ARM64 sender with a pinned content hash."""
import hashlib
import os
from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
directory = root / '.hori-build/airplay-cli-v0.5.8'
target = directory / 'hori-airplay'
digest = '6d460ad08562844d978f337bee28a79b0eea390c84c239b5d66f55624e27d719'
directory.mkdir(parents=True, exist_ok=True)
if target.exists():
    if hashlib.sha256(target.read_bytes()).hexdigest() != digest:
        raise SystemExit('Native AirPlay sender checksum differs; refusing to execute or replace it.')
else:
    with tempfile.NamedTemporaryFile(dir=directory, delete=False) as temporary:
        path = Path(temporary.name)
    try:
        subprocess.run(['curl', '--fail', '--location', '--silent', '--show-error',
            'https://github.com/music-assistant/airplay-cli/releases/download/v0.5.8/cliairplay-macos-arm64',
            '--output', str(path)], check=True)
        if hashlib.sha256(path.read_bytes()).hexdigest() != digest:
            raise SystemExit('Official AirPlay download checksum failed.')
        path.chmod(0o755)
        os.replace(path, target)
    finally:
        path.unlink(missing_ok=True)
print('Verified unchanged official airplay-cli v0.5.8 ARM64 sender.')
