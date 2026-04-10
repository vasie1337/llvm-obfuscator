#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
PASS_LIB="$PROJECT_DIR/build/passes/obfuscator.so"
IR_DIR="$SCRIPT_DIR/ir"
OPT=opt-20
FILECHECK=FileCheck-20

if [ ! -f "$PASS_LIB" ]; then
    echo "ERROR: Pass library not found at $PASS_LIB"
    echo "Run: cd build && cmake -G Ninja -DLLVM_DIR=/usr/lib/llvm-20/lib/cmake/llvm .. && ninja"
    exit 1
fi

if ! command -v "$FILECHECK" &>/dev/null; then
    FILECHECK=FileCheck
    if ! command -v "$FILECHECK" &>/dev/null; then
        echo "ERROR: FileCheck not found (tried FileCheck-20 and FileCheck)"
        echo "Install: sudo apt install llvm-20-tools"
        exit 1
    fi
fi

if ! command -v "$OPT" &>/dev/null; then
    echo "ERROR: $OPT not found"
    exit 1
fi

PASS=0
FAIL=0
TOTAL=0

for ll in "$IR_DIR"/*.ll; do
    [ -f "$ll" ] || continue
    name=$(basename "$ll" .ll)
    TOTAL=$((TOTAL + 1))

    passes=$(grep '^; PASS:' "$ll" | head -1 | sed 's/^; PASS: *//')
    if [ -z "$passes" ]; then
        echo "SKIP: $name (no ; PASS: directive found)"
        continue
    fi

    if $OPT --load-pass-plugin="$PASS_LIB" --passes="$passes" \
            -S "$ll" 2>/dev/null | $FILECHECK "$ll" 2>/dev/null; then
        echo "PASS: $name"
        PASS=$((PASS + 1))
    else
        echo "FAIL: $name"
        FAIL=$((FAIL + 1))
    fi
done

echo ""
echo "IR test results: $PASS/$TOTAL passed, $FAIL failed"
exit $FAIL
