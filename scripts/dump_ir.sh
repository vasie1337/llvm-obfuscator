#!/bin/bash
set -euo pipefail

# Emit LLVM IR before and after obfuscation for inspection.
#
# Usage:
#   ./scripts/dump_ir.sh <source.c> [-p passes]
#
# Writes <basename>.ll (original) and <basename>_obfs.ll (obfuscated) to cwd.

PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
PASS_LIB="$PROJECT_DIR/build/passes/Obfuscator.so"
CLANG=clang-20
OPT=opt-20

PASSES="instsub,mbasub,bcf"
SOURCE=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        -p|--passes) PASSES="$2"; shift 2 ;;
        -h|--help)
            echo "Usage: $0 <source.c> [-p passes]"
            echo "Dumps original and obfuscated LLVM IR to current directory."
            exit 0
            ;;
        *) SOURCE="$1"; shift ;;
    esac
done

if [[ -z "$SOURCE" ]]; then
    echo "ERROR: no source file specified" >&2; exit 1
fi
if [[ ! -f "$PASS_LIB" ]]; then
    echo "ERROR: Obfuscator.so not found. Run ./scripts/build.sh first." >&2; exit 1
fi

BASENAME=$(basename "$SOURCE" .c)

echo "Emitting original IR..."
$CLANG -O0 -Xclang -disable-O0-optnone -emit-llvm -S \
    "$SOURCE" -o "${BASENAME}.ll"

echo "Applying passes: $PASSES"
$OPT --load-pass-plugin="$PASS_LIB" --passes="$PASSES" \
    -S "${BASENAME}.ll" -o "${BASENAME}_obfs.ll"

echo ""
echo "Original:   ${BASENAME}.ll"
echo "Obfuscated: ${BASENAME}_obfs.ll"
