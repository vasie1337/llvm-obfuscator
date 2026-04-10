#!/bin/bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$PROJECT_DIR/build"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake -G Ninja \
    -DLLVM_DIR=/usr/lib/llvm-20/lib/cmake/llvm \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    "$PROJECT_DIR"

ninja

echo ""
echo "Build complete: $BUILD_DIR/passes/Obfuscator.so"
