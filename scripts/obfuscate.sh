#!/bin/bash
set -euo pipefail

# Obfuscate a C source file with selected passes.
#
# Usage:
#   ./scripts/obfuscate.sh <source.c> [-o output] [-p passes] [--target triple]
#
# Examples:
#   ./scripts/obfuscate.sh test-bins/src/demo.c
#   ./scripts/obfuscate.sh test-bins/src/demo.c -p bcf,instsub -o demo_bcf
#   ./scripts/obfuscate.sh test-bins/src/demo.c --target x86_64-w64-mingw32 -o demo.exe

PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
PASS_LIB="$PROJECT_DIR/build/passes/obfuscator.so"
CLANG=clang-20
OPT=opt-20
LLC=llc-20

PASSES="instsub,mbasub,bcf"
OUTPUT=""
TARGET=""
SOURCE=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        -p|--passes) PASSES="$2"; shift 2 ;;
        -o|--output) OUTPUT="$2"; shift 2 ;;
        --target)    TARGET="$2"; shift 2 ;;
        -h|--help)
            echo "Usage: $0 <source.c> [-o output] [-p passes] [--target triple]"
            echo ""
            echo "Passes (comma-separated):"
            echo "  instsub   Instruction substitution (a+b -> a-(-b), etc.)"
            echo "  mbasub    Mixed boolean-arithmetic substitution"
            echo "  bcf       Bogus control flow"
            echo "  cff       Control flow flattening"
            echo ""
            echo "Targets:"
            echo "  (default)                  Native Linux ELF"
            echo "  x86_64-w64-mingw32         Windows PE (x86-64)"
            echo ""
            echo "Examples:"
            echo "  $0 test-bins/src/demo.c"
            echo "  $0 test-bins/src/demo.c -p bcf,cff -o out.exe --target x86_64-w64-mingw32"
            exit 0
            ;;
        *)
            if [[ -z "$SOURCE" ]]; then
                SOURCE="$1"; shift
            else
                echo "ERROR: unexpected argument '$1'" >&2; exit 1
            fi
            ;;
    esac
done

if [[ -z "$SOURCE" ]]; then
    echo "ERROR: no source file specified (use -h for help)" >&2
    exit 1
fi

if [[ ! -f "$PASS_LIB" ]]; then
    echo "ERROR: obfuscator.so not found. Run ./scripts/build.sh first." >&2
    exit 1
fi

BASENAME=$(basename "$SOURCE" .c)
[[ -z "$OUTPUT" ]] && OUTPUT="${BASENAME}_obfs"

TMPDIR=$(mktemp -d)
trap "rm -rf $TMPDIR" EXIT

if [[ -n "$TARGET" ]]; then
    # Cross-compilation: clang IR -> opt -> llc -> mingw linker
    echo "[1/4] Emitting IR (target: $TARGET)..."
    $CLANG -O0 -Xclang -disable-O0-optnone -emit-llvm -S \
        --target="$TARGET" "$SOURCE" -o "$TMPDIR/input.ll"

    echo "[2/4] Applying passes: $PASSES"
    $OPT --load-pass-plugin="$PASS_LIB" --passes="$PASSES" \
        -S "$TMPDIR/input.ll" -o "$TMPDIR/obfs.ll"

    echo "[3/4] Compiling to object..."
    $LLC -O0 -filetype=obj -mtriple="$TARGET" \
        "$TMPDIR/obfs.ll" -o "$TMPDIR/obfs.o"

    echo "[4/4] Linking -> $OUTPUT"
    case "$TARGET" in
        *mingw*) x86_64-w64-mingw32-gcc "$TMPDIR/obfs.o" -o "$OUTPUT" ;;
        *)       $CLANG --target="$TARGET" "$TMPDIR/obfs.o" -o "$OUTPUT" ;;
    esac
else
    # Native: clang IR -> opt -> clang
    echo "[1/3] Emitting IR..."
    $CLANG -O0 -Xclang -disable-O0-optnone -emit-llvm -S \
        "$SOURCE" -o "$TMPDIR/input.ll"

    echo "[2/3] Applying passes: $PASSES"
    $OPT --load-pass-plugin="$PASS_LIB" --passes="$PASSES" \
        -S "$TMPDIR/input.ll" -o "$TMPDIR/obfs.ll"

    echo "[3/3] Compiling -> $OUTPUT"
    $CLANG -O0 -x ir "$TMPDIR/obfs.ll" -o "$OUTPUT"
fi

echo "Done: $OUTPUT"
ls -lh "$OUTPUT"
