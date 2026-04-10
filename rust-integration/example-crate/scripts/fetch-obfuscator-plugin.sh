#!/usr/bin/env bash
# Download the standalone pass plugin from GitHub Releases into vendor/obfuscator.so
# (same layout expected by .cargo/config.toml).
#
# Usage:
#   ./scripts/fetch-obfuscator-plugin.sh [TAG]
# TAG is a release tag (e.g. v0.1.0) or the word "latest".
#
# The repository is taken from GITHUB_REPOSITORY (owner/name), or from `git remote origin`
# at the llvm-obfuscator repo root.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CRATE_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"
VENDOR_DIR="${CRATE_ROOT}/vendor"
OUT="${VENDOR_DIR}/obfuscator.so"
ASSET_NAME="obfuscator-x86_64-unknown-linux-gnu.so"

TAG="${1:-latest}"

detect_repo() {
  if [[ -n "${GITHUB_REPOSITORY:-}" ]]; then
    printf '%s' "${GITHUB_REPOSITORY}"
    return 0
  fi
  if git -C "${REPO_ROOT}" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    git -C "${REPO_ROOT}" remote get-url origin 2>/dev/null \
      | sed -E 's#^(git@github\.com:|https://github\.com/)##; s#\.git$##' \
      | tr -d '\n' \
      || true
  fi
}

REPO="$(detect_repo)"
if [[ -z "${REPO}" ]]; then
  echo "Could not determine GitHub repo. Set GITHUB_REPOSITORY=owner/name and retry." >&2
  exit 1
fi

if [[ "${TAG}" == "latest" ]]; then
  URL="https://github.com/${REPO}/releases/latest/download/${ASSET_NAME}"
else
  URL="https://github.com/${REPO}/releases/download/${TAG}/${ASSET_NAME}"
fi

mkdir -p "${VENDOR_DIR}"
echo "Downloading ${URL}" >&2
curl -fsSL -o "${OUT}" "${URL}"
echo "Wrote ${OUT}" >&2
