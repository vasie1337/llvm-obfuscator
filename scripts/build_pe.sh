#!/bin/bash
set -euo pipefail

# Build Windows PE binaries: clean + each pass variant + combined.
#
# Usage:
#   ./scripts/build_pe.sh <source.c> [-d outdir]
#
# Example:
#   ./scripts/build_pe.sh test-bins/src/demo.c -d test-bins/bin

PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
PASS_LIB="$PROJECT_DIR/build/passes/obfuscator.so"
CLANG=clang-20
OPT=opt-20
LLC=llc-20
MINGW_GCC=x86_64-w64-mingw32-gcc
TRIPLE=x86_64-w64-mingw32

SOURCE=""
OUTDIR="."

while [[ $# -gt 0 ]]; do
    case "$1" in
        -d|--outdir) OUTDIR="$2"; shift 2 ;;
        -h|--help)
            echo "Usage: $0 <source.c> [-d outdir]"
            echo "Builds clean and obfuscated Windows PE variants."
            exit 0
            ;;
        *) SOURCE="$1"; shift ;;
    esac
done

if [[ -z "$SOURCE" ]]; then
    echo "ERROR: no source file specified" >&2; exit 1
fi
if [[ ! -f "$PASS_LIB" ]]; then
    echo "ERROR: obfuscator.so not found. Run ./scripts/build.sh first." >&2; exit 1
fi
if ! command -v "$MINGW_GCC" &>/dev/null; then
    echo "ERROR: $MINGW_GCC not found. Install mingw-w64." >&2; exit 1
fi

BASENAME=$(basename "$SOURCE" .c)
mkdir -p "$OUTDIR"

TMPDIR=$(mktemp -d)
trap "rm -rf $TMPDIR" EXIT

build_variant() {
    local name="$1" passes="$2"
    local outfile="$OUTDIR/${BASENAME}_${name}.exe"

    if [[ "$passes" == "none" ]]; then
        $LLC -O0 -filetype=obj -mtriple=$TRIPLE "$TMPDIR/input.ll" -o "$TMPDIR/${name}.o"
    else
        $OPT --load-pass-plugin="$PASS_LIB" --passes="$passes" \
            -S "$TMPDIR/input.ll" -o "$TMPDIR/${name}.ll"
        $LLC -O0 -filetype=obj -mtriple=$TRIPLE "$TMPDIR/${name}.ll" -o "$TMPDIR/${name}.o"
    fi

    $MINGW_GCC "$TMPDIR/${name}.o" -o "$outfile"
    printf "  %-40s %s\n" "$outfile" "($passes)"
}

echo "Emitting IR (target: $TRIPLE)..."
$CLANG -O0 -Xclang -disable-O0-optnone -emit-llvm -S \
    --target=$TRIPLE "$SOURCE" -o "$TMPDIR/input.ll"

echo "Building variants:"
build_variant "clean"      "none"
build_variant "obfuscated" "function(instsub,mbasub,simd,constunfold,cff,bcf,bbfission<junk=6;max=512>),strenc"

echo ""
echo "Done:"
ls -lh "$OUTDIR/${BASENAME}_"*.exe
