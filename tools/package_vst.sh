#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
PROJECT_DIR="$ROOT_DIR/vst-juce"
BUILD_DIR="$PROJECT_DIR/build"
DIST_DIR="$ROOT_DIR/dist"

EXTRA_CMAKE_ARGS=()
if [[ -n "${JUCE_SOURCE_DIR:-}" ]]; then
  EXTRA_CMAKE_ARGS+=("-DJUCE_SOURCE_DIR=$JUCE_SOURCE_DIR")
fi

cmake -S "$PROJECT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release "${EXTRA_CMAKE_ARGS[@]}"
cmake --build "$BUILD_DIR" --config Release

VST_PATH="$(find "$BUILD_DIR" -type f -name 'HulusiVST.vst3' | head -n 1 || true)"
if [[ -z "$VST_PATH" ]]; then
  echo "[ERROR] HulusiVST.vst3 not found after build."
  exit 1
fi

mkdir -p "$DIST_DIR"
ZIP_PATH="$DIST_DIR/hulusi-vst3-$(uname -s | tr '[:upper:]' '[:lower:]').zip"

# .vst3 在部分平台是 bundle 目录，因此 zip 需要递归
python3 - <<PY
import os, zipfile
vst_path = r'''$VST_PATH'''
zip_path = r'''$ZIP_PATH'''
base = os.path.dirname(vst_path)
with zipfile.ZipFile(zip_path, 'w', zipfile.ZIP_DEFLATED) as z:
    if os.path.isdir(vst_path):
        for root, _, files in os.walk(vst_path):
            for f in files:
                p = os.path.join(root, f)
                z.write(p, os.path.relpath(p, base))
    else:
        z.write(vst_path, os.path.basename(vst_path))
print(zip_path)
PY

echo "[OK] packaged: $ZIP_PATH"
