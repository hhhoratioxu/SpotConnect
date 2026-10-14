#!/bin/bash
set -euo pipefail
hori_root=$(cd "$(dirname "$0")/.." && pwd)
export DEVELOPER_DIR=/Library/Developer/CommandLineTools
export PATH="$hori_root/.hori-build/tools/bin:$PATH"
hori_python=${HORI_PYTHON:-"$hori_root/.hori-build/tools/bin/python"}
exec "$hori_python" "$hori_root/scripts/package.py"
