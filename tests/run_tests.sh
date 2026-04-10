#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
PASS_LIB="$PROJECT_DIR/build/passes/Obfuscator.so"
TEST_SRC="$PROJECT_DIR/test-bins/src"
CLANG=clang-20
OPT=opt-20
RUSTC=rustc
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

compare_outputs() {
    local name="$1" orig_bin="$2" obfs_bin="$3"

    orig_out=$("$orig_bin" 2>&1) || true
    orig_rc=${PIPESTATUS[0]:-$?}

    obfs_out=$("$obfs_bin" 2>&1) || true
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
}

# --- C tests ---

for src in "$TEST_SRC"/*.c; do
    [ -f "$src" ] || continue
    name=$(basename "$src" .c)
    TOTAL=$((TOTAL + 1))

    $CLANG -O0 -o "$TMPDIR/${name}_orig" "$src" 2>/dev/null

    if ! $CLANG -fpass-plugin="$PASS_LIB" -O1 -o "$TMPDIR/${name}_obfs" "$src" 2>/dev/null; then
        echo "FAIL: $name (compilation with pass failed)"
        FAIL=$((FAIL + 1))
        continue
    fi

    compare_outputs "$name" "$TMPDIR/${name}_orig" "$TMPDIR/${name}_obfs"
done

# --- C tests: MBA substitution only (via opt) ---

for src in "$TEST_SRC"/*.c; do
    [ -f "$src" ] || continue
    name="mbasub_$(basename "$src" .c)"
    TOTAL=$((TOTAL + 1))

    $CLANG -O0 -o "$TMPDIR/${name}_orig" "$src" 2>/dev/null
    if ! $CLANG -O0 -emit-llvm -S "$src" -o "$TMPDIR/${name}.ll" 2>/dev/null; then
        echo "FAIL: $name (emit IR failed)"
        FAIL=$((FAIL + 1))
        continue
    fi

    if ! $OPT --load-pass-plugin="$PASS_LIB" --passes="mbasub" \
            -S "$TMPDIR/${name}.ll" -o "$TMPDIR/${name}_obfs.ll" 2>/dev/null; then
        echo "FAIL: $name (opt mbasub pass failed)"
        FAIL=$((FAIL + 1))
        continue
    fi

    if ! $CLANG -O0 -x ir "$TMPDIR/${name}_obfs.ll" -o "$TMPDIR/${name}_obfs" 2>/dev/null; then
        echo "FAIL: $name (clang compile obfuscated IR failed)"
        FAIL=$((FAIL + 1))
        continue
    fi

    compare_outputs "$name" "$TMPDIR/${name}_orig" "$TMPDIR/${name}_obfs"
done

# --- C tests: bogus control flow only (via opt) ---

for src in "$TEST_SRC"/*.c; do
    [ -f "$src" ] || continue
    name="bcf_$(basename "$src" .c)"
    TOTAL=$((TOTAL + 1))

    $CLANG -O0 -o "$TMPDIR/${name}_orig" "$src" 2>/dev/null
    if ! $CLANG -O0 -emit-llvm -S "$src" -o "$TMPDIR/${name}.ll" 2>/dev/null; then
        echo "FAIL: $name (emit IR failed)"
        FAIL=$((FAIL + 1))
        continue
    fi

    if ! $OPT --load-pass-plugin="$PASS_LIB" --passes="bcf" \
            -S "$TMPDIR/${name}.ll" -o "$TMPDIR/${name}_obfs.ll" 2>/dev/null; then
        echo "FAIL: $name (opt bcf pass failed)"
        FAIL=$((FAIL + 1))
        continue
    fi

    if ! $CLANG -O0 -x ir "$TMPDIR/${name}_obfs.ll" -o "$TMPDIR/${name}_obfs" 2>/dev/null; then
        echo "FAIL: $name (clang compile obfuscated IR failed)"
        FAIL=$((FAIL + 1))
        continue
    fi

    compare_outputs "$name" "$TMPDIR/${name}_orig" "$TMPDIR/${name}_obfs"
done

# --- C tests: control flow flattening only (via opt) ---

for src in "$TEST_SRC"/*.c; do
    [ -f "$src" ] || continue
    name="cff_$(basename "$src" .c)"
    TOTAL=$((TOTAL + 1))

    $CLANG -O0 -o "$TMPDIR/${name}_orig" "$src" 2>/dev/null
    if ! $CLANG -O0 -emit-llvm -S "$src" -o "$TMPDIR/${name}.ll" 2>/dev/null; then
        echo "FAIL: $name (emit IR failed)"
        FAIL=$((FAIL + 1))
        continue
    fi

    if ! $OPT --load-pass-plugin="$PASS_LIB" --passes="cff" \
            -S "$TMPDIR/${name}.ll" -o "$TMPDIR/${name}_obfs.ll" 2>/dev/null; then
        echo "FAIL: $name (opt cff pass failed)"
        FAIL=$((FAIL + 1))
        continue
    fi

    if ! $CLANG -O0 -x ir "$TMPDIR/${name}_obfs.ll" -o "$TMPDIR/${name}_obfs" 2>/dev/null; then
        echo "FAIL: $name (clang compile obfuscated IR failed)"
        FAIL=$((FAIL + 1))
        continue
    fi

    compare_outputs "$name" "$TMPDIR/${name}_orig" "$TMPDIR/${name}_obfs"
done

# --- Rust tests (no_std, IR pipeline through opt-20) ---
#
# Pipeline: rustc --emit=llvm-ir -> opt-20 (apply pass) -> clang-20 (link)
# Uses no_std Rust sources with libc FFI so clang can link them without
# the Rust runtime. LLVM IR text format is used for cross-version tolerance
# (rustc's LLVM version may differ from the pass's LLVM 20).

HAS_RUST=true
if ! command -v "$RUSTC" &>/dev/null; then
    echo "SKIP: rustc not found, skipping Rust tests"
    HAS_RUST=false
fi
if ! command -v "$OPT" &>/dev/null; then
    echo "SKIP: $OPT not found, skipping Rust tests"
    HAS_RUST=false
fi

if $HAS_RUST; then
    rust_sources=("$TEST_SRC"/*.rs)
    if [ -f "${rust_sources[0]:-}" ]; then
        for src in "${rust_sources[@]}"; do
            [ -f "$src" ] || continue
            name="rust_$(basename "$src" .rs)"
            TOTAL=$((TOTAL + 1))

            # Emit LLVM IR from rustc (no_std, single codegen unit for one .ll file).
            # opt-level=1 eliminates unreachable panic paths (overflow/div-by-zero
            # checks on constants) that would otherwise need Rust runtime symbols.
            if ! $RUSTC --edition 2021 \
                    --emit=llvm-ir \
                    -C panic=abort \
                    -C opt-level=1 \
                    -C codegen-units=1 \
                    -o "$TMPDIR/${name}.ll" \
                    "$src" 2>/dev/null; then
                echo "FAIL: $name (rustc emit IR failed)"
                FAIL=$((FAIL + 1))
                continue
            fi

            # Original: compile IR directly with clang
            if ! $CLANG -O0 -x ir "$TMPDIR/${name}.ll" -o "$TMPDIR/${name}_orig" -lc 2>/dev/null; then
                echo "FAIL: $name (clang compile original IR failed)"
                FAIL=$((FAIL + 1))
                continue
            fi

            # Obfuscated: run pass via opt-20, then compile with clang
            if ! $OPT --load-pass-plugin="$PASS_LIB" --passes="instsub" \
                    -S "$TMPDIR/${name}.ll" -o "$TMPDIR/${name}_obfs.ll" 2>/dev/null; then
                echo "FAIL: $name (opt-20 pass failed -- possible LLVM version mismatch)"
                FAIL=$((FAIL + 1))
                continue
            fi

            if ! $CLANG -O0 -x ir "$TMPDIR/${name}_obfs.ll" -o "$TMPDIR/${name}_obfs" -lc 2>/dev/null; then
                echo "FAIL: $name (clang compile obfuscated IR failed)"
                FAIL=$((FAIL + 1))
                continue
            fi

            compare_outputs "$name" "$TMPDIR/${name}_orig" "$TMPDIR/${name}_obfs"
        done
    fi
fi

echo ""
echo "Results: $PASS/$TOTAL passed, $FAIL failed"
exit $FAIL
