#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
PASS_LIB="$PROJECT_DIR/build/passes/Obfuscator.so"
TEST_SRC="$PROJECT_DIR/test-bins/src"
CLANG=clang-20
TMPDIR=$(mktemp -d)
trap "rm -rf $TMPDIR" EXIT

if [ ! -f "$PASS_LIB" ]; then
    echo "ERROR: Pass library not found at $PASS_LIB"
    echo "Run: cd build && cmake -G Ninja -DLLVM_DIR=/usr/lib/llvm-20/lib/cmake/llvm .. && ninja"
    exit 1
fi

PASS=0
FAIL=0
TOTAL=0

for src in "$TEST_SRC"/*.c; do
    name=$(basename "$src" .c)
    TOTAL=$((TOTAL + 1))

    # Compile original (no obfuscation)
    $CLANG -O0 -o "$TMPDIR/${name}_orig" "$src" 2>/dev/null

    # Compile with obfuscation pass
    if ! $CLANG -fpass-plugin="$PASS_LIB" -O1 -o "$TMPDIR/${name}_obfs" "$src" 2>/dev/null; then
        echo "FAIL: $name (compilation with pass failed)"
        FAIL=$((FAIL + 1))
        continue
    fi

    # Run both and capture output + exit code
    orig_out=$("$TMPDIR/${name}_orig" 2>&1) || true
    orig_rc=${PIPESTATUS[0]:-$?}

    obfs_out=$("$TMPDIR/${name}_obfs" 2>&1) || true
    obfs_rc=${PIPESTATUS[0]:-$?}

    if [ "$orig_out" = "$obfs_out" ] && [ "$orig_rc" = "$obfs_rc" ]; then
        echo "PASS: $name"
        PASS=$((PASS + 1))
    else
        echo "FAIL: $name"
        if [ "$orig_rc" != "$obfs_rc" ]; then
            echo "  exit code: expected=$orig_rc got=$obfs_rc"
        fi
        if [ "$orig_out" != "$obfs_out" ]; then
            echo "  stdout differs"
            diff <(echo "$orig_out") <(echo "$obfs_out") | head -10 || true
        fi
        FAIL=$((FAIL + 1))
    fi
done

echo ""
echo "Results: $PASS/$TOTAL passed, $FAIL failed"
exit $FAIL
