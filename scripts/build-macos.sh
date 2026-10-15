#!/bin/bash
set -euo pipefail
hori_root=$(cd "$(dirname "$0")/.." && pwd)
if [[ $(uname -s) != Darwin || $(uname -m) != arm64 ]]; then
  echo "HoriSpotConnect Preview requires macOS Apple Silicon." >&2; exit 2
fi
if [[ -d /Library/Developer/CommandLineTools ]]; then
  export DEVELOPER_DIR=/Library/Developer/CommandLineTools
fi
hori_work="$hori_root/.hori-build"
mkdir -p "$hori_work"
hori_python=${HORI_PYTHON:-python3}
if [[ ! -x "$hori_work/tools/bin/python" ]]; then
  "$hori_python" -m venv "$hori_work/tools"
fi
export PATH="$hori_work/tools/bin:$PATH"
export PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION=python
if [[ ! -f "$hori_work/tools/.hori-pins-v1" ]]; then
  python -m pip install --disable-pip-version-check \
    'cmake==3.31.10' 'ninja==1.13.2' 'setuptools==80.9.0' \
    'grpcio-tools==1.71.0' 'protobuf==5.29.5'
  touch "$hori_work/tools/.hori-pins-v1"
fi
cat > "$hori_work/tools/bin/protoc" <<'SH'
#!/bin/sh
exec "$(dirname "$0")/python" -m grpc_tools.protoc "$@"
SH
chmod +x "$hori_work/tools/bin/protoc"

# Preserve the pinned submodule commits, including the cspot fork.
git -C "$hori_root" submodule update --init --recursive --filter=blob:none \
  common/cspot common/crosstools common/libcodecs common/libpupnp common/libmbedtls \
  spotraop/libraop spotraop/http-fetcher spotraop/libopenssl spotraop/libmdns

hori_prefix="$hori_work/deps"
if [[ ! -f "$hori_prefix/lib/libFLAC.a" ]]; then
  hori_tar="$hori_work/flac-1.5.0.tar.xz"
  curl --fail --location --silent --show-error \
    https://github.com/xiph/flac/releases/download/1.5.0/flac-1.5.0.tar.xz -o "$hori_tar"
  printf '%s  %s\n' f2c1c76592a82ffff8413ba3c4a1299b6c7ab06c734dee03fd88630485c2b920 "$hori_tar" | shasum -a 256 -c -
  tar -xf "$hori_tar" -C "$hori_work"
  cmake -S "$hori_work/flac-1.5.0" -B "$hori_work/flac-build" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64 -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
    -DCMAKE_INSTALL_PREFIX="$hori_prefix" -DBUILD_SHARED_LIBS=OFF -DWITH_OGG=OFF \
    -DBUILD_TESTING=OFF -DBUILD_EXAMPLES=OFF -DBUILD_CXXLIBS=OFF -DINSTALL_MANPAGES=OFF
  cmake --build "$hori_work/flac-build" --parallel 8
  cmake --install "$hori_work/flac-build" > "$hori_work/flac-install.log"
fi

hori_kind=native
hori_sanitize=OFF
if [[ ${1:-} == --sanitize ]]; then hori_kind=native-asan; hori_sanitize=ON
elif [[ $# != 0 ]]; then echo "usage: build-macos.sh [--sanitize]" >&2; exit 2; fi
hori_build="$hori_work/$hori_kind"
cmake -S "$hori_root/macos" -B "$hori_build" -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_PREFIX_PATH="$hori_prefix" -DHORI_SANITIZE="$hori_sanitize" \
  -DBELL_EXTERNAL_MBEDTLS="$hori_root/common/libmbedtls" -DMBEDTLS_RELEASE=RELEASE
cmake --build "$hori_build" --target spotraop-macos-arm64-static hori-discover \
  hori-audio-probe hori-raop-probe audio-tests hori-native-tests hori-network-tests hori-alac-tests --parallel 8
ctest --test-dir "$hori_build" --output-on-failure --no-tests=error \
  --output-junit "$hori_build/test-results.xml"
echo "Verified native build: $hori_build"
