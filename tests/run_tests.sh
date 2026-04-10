#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
PASS_LIB="$PROJECT_DIR/build/passes/obfuscator.so"
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

TIMEOUT=10

run_with_timeout() {
    local bin="$1" out_file="$2"
    timeout "$TIMEOUT" "$bin" > "$out_file" 2>&1
    return $?
}

compare_outputs() {
    local name="$1" orig_bin="$2" obfs_bin="$3"
    local orig_out_file="$TMPDIR/orig_out.txt"
    local obfs_out_file="$TMPDIR/obfs_out.txt"

    local orig_rc=0 obfs_rc=0
    run_with_timeout "$orig_bin" "$orig_out_file" || orig_rc=$?
    run_with_timeout "$obfs_bin" "$obfs_out_file" || obfs_rc=$?

    if [ "$orig_rc" = "124" ]; then
        echo "FAIL: $name (original binary timed out after ${TIMEOUT}s)"
        FAIL=$((FAIL + 1))
        return
    fi

    if [ "$obfs_rc" = "124" ]; then
        echo "FAIL: $name (obfuscated binary timed out after ${TIMEOUT}s)"
        FAIL=$((FAIL + 1))
        return
    fi

    if [ "$obfs_rc" -gt 128 ] && [ "$orig_rc" -le 128 ]; then
        echo "FAIL: $name (obfuscated binary crashed with signal $((obfs_rc - 128)))"
        FAIL=$((FAIL + 1))
        return
    fi

    local orig_out obfs_out
    orig_out=$(cat "$orig_out_file")
    obfs_out=$(cat "$obfs_out_file")

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

check_ir_marker() {
    local name="$1" ir_file="$2" pattern="$3" description="$4"
    TOTAL=$((TOTAL + 1))
    if grep -q "$pattern" "$ir_file" 2>/dev/null; then
        echo "PASS: $name (IR marker: $description)"
        PASS=$((PASS + 1))
    else
        echo "FAIL: $name (IR marker: expected '$description' in obfuscated IR)"
        FAIL=$((FAIL + 1))
    fi
}

# Emit IR suitable for opt: -O1 with LLVM passes disabled strips the
# optnone attribute (which -O0 adds and which makes opt skip function
# passes) while keeping the IR unoptimized.
emit_ir() {
    local src="$1" out="$2"
    $CLANG -O1 -Xclang -disable-llvm-passes -emit-llvm -S "$src" -o "$out" 2>/dev/null
}

# ============================================================================
# C tests: combined plugin at -O1
# ============================================================================

echo "=== C tests: combined plugin (-O1) ==="

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

# ============================================================================
# Helper: run a single per-pass opt test for C sources
# ============================================================================

run_c_opt_test() {
    local pass_prefix="$1" pass_flag="$2" src="$3"
    local name="${pass_prefix}_$(basename "$src" .c)"
    TOTAL=$((TOTAL + 1))

    $CLANG -O0 -o "$TMPDIR/${name}_orig" "$src" 2>/dev/null
    if ! emit_ir "$src" "$TMPDIR/${name}.ll"; then
        echo "FAIL: $name (emit IR failed)"
        FAIL=$((FAIL + 1))
        return
    fi

    if ! $OPT --load-pass-plugin="$PASS_LIB" --passes="$pass_flag" \
            -S "$TMPDIR/${name}.ll" -o "$TMPDIR/${name}_obfs.ll" 2>/dev/null; then
        echo "FAIL: $name (opt $pass_prefix pass failed)"
        FAIL=$((FAIL + 1))
        return
    fi

    if ! $CLANG -O0 -x ir "$TMPDIR/${name}_obfs.ll" -o "$TMPDIR/${name}_obfs" 2>/dev/null; then
        echo "FAIL: $name (clang compile obfuscated IR failed)"
        FAIL=$((FAIL + 1))
        return
    fi

    compare_outputs "$name" "$TMPDIR/${name}_orig" "$TMPDIR/${name}_obfs"
}

# ============================================================================
# C tests: instruction substitution only (via opt)
# ============================================================================

echo ""
echo "=== C tests: instsub (via opt) ==="

for src in "$TEST_SRC"/*.c; do
    [ -f "$src" ] || continue
    run_c_opt_test "instsub" "instsub" "$src"
done

# ============================================================================
# C tests: MBA substitution only (via opt)
# ============================================================================

echo ""
echo "=== C tests: mbasub (via opt) ==="

for src in "$TEST_SRC"/*.c; do
    [ -f "$src" ] || continue
    run_c_opt_test "mbasub" "mbasub" "$src"
done

# ============================================================================
# C tests: bogus control flow only (via opt)
# ============================================================================

echo ""
echo "=== C tests: bcf (via opt) ==="

for src in "$TEST_SRC"/*.c; do
    [ -f "$src" ] || continue
    run_c_opt_test "bcf" "bcf" "$src"
done

# ============================================================================
# C tests: control flow flattening only (via opt)
# ============================================================================

echo ""
echo "=== C tests: cff (via opt) ==="

for src in "$TEST_SRC"/*.c; do
    [ -f "$src" ] || continue
    run_c_opt_test "cff" "cff" "$src"
done

# ============================================================================
# C tests: string encryption only (via opt)
# ============================================================================

echo ""
echo "=== C tests: strenc (via opt) ==="

for src in "$TEST_SRC"/*.c; do
    [ -f "$src" ] || continue
    run_c_opt_test "strenc" "strenc" "$src"
done

# ============================================================================
# C tests: all passes combined (via opt)
# ============================================================================

echo ""
echo "=== C tests: combined passes (via opt) ==="

for src in "$TEST_SRC"/*.c; do
    [ -f "$src" ] || continue
    run_c_opt_test "combined" "function(instsub,mbasub,simd,constunfold,cff,bcf),strenc" "$src"
done

# ============================================================================
# IR marker verification: confirm transforms actually fired
# ============================================================================

echo ""
echo "=== IR marker verification ==="

MARKER_SRC="$TEST_SRC/demo.c"
if [ -f "$MARKER_SRC" ]; then
    emit_ir "$MARKER_SRC" "$TMPDIR/marker.ll"

    $OPT --load-pass-plugin="$PASS_LIB" --passes="cff" \
        -S "$TMPDIR/marker.ll" -o "$TMPDIR/marker_cff.ll" 2>/dev/null
    check_ir_marker "verify_cff" "$TMPDIR/marker_cff.ll" "cff.dispatch" \
        "CFF dispatcher block"

    $OPT --load-pass-plugin="$PASS_LIB" --passes="bcf" \
        -S "$TMPDIR/marker.ll" -o "$TMPDIR/marker_bcf.ll" 2>/dev/null
    check_ir_marker "verify_bcf" "$TMPDIR/marker_bcf.ll" "bcf_opaque" \
        "BCF opaque predicate global"
    check_ir_marker "verify_bcf_bogus" "$TMPDIR/marker_bcf.ll" ".bogus" \
        "BCF bogus block labels"

    $OPT --load-pass-plugin="$PASS_LIB" --passes="strenc" \
        -S "$TMPDIR/marker.ll" -o "$TMPDIR/marker_strenc.ll" 2>/dev/null
    check_ir_marker "verify_strenc" "$TMPDIR/marker_strenc.ll" "__strenc_ctor" \
        "StringEncryption decryption constructor"
fi

NOSTR_SRC="$TEST_SRC/edge_nostring.c"
if [ -f "$NOSTR_SRC" ]; then
    emit_ir "$NOSTR_SRC" "$TMPDIR/nostr.ll"
    $OPT --load-pass-plugin="$PASS_LIB" --passes="strenc" \
        -S "$TMPDIR/nostr.ll" -o "$TMPDIR/nostr_strenc.ll" 2>/dev/null
    TOTAL=$((TOTAL + 1))
    if grep -q "__strenc_ctor" "$TMPDIR/nostr_strenc.ll" 2>/dev/null; then
        echo "FAIL: verify_strenc_noop (strenc should not fire on string-free program)"
        FAIL=$((FAIL + 1))
    else
        echo "PASS: verify_strenc_noop (strenc correctly skipped string-free program)"
        PASS=$((PASS + 1))
    fi
fi

# ============================================================================
# Compilation stress test: combined passes at -O2 and -O3
# ============================================================================

echo ""
echo "=== Compilation stress tests (-O2, -O3) ==="

for opt_level in O2 O3; do
    for src in "$TEST_SRC"/*.c; do
        [ -f "$src" ] || continue
        name="stress_${opt_level}_$(basename "$src" .c)"
        TOTAL=$((TOTAL + 1))

        if $CLANG -fpass-plugin="$PASS_LIB" -"$opt_level" \
                -o "$TMPDIR/${name}_obfs" "$src" 2>/dev/null; then
            echo "PASS: $name (compiles)"
            PASS=$((PASS + 1))
        else
            echo "FAIL: $name (compilation crashed at -$opt_level)"
            FAIL=$((FAIL + 1))
        fi
    done
done

# ============================================================================
# Summary
# ============================================================================

echo ""
echo "Results: $PASS/$TOTAL passed, $FAIL failed"
if [ "$FAIL" -gt 0 ]; then exit 1; else exit 0; fi
