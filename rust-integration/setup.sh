#!/usr/bin/env bash
# One-shot: download the standalone plugin into example-crate/vendor and print next steps.
# Run from anywhere:  bash rust-integration/setup.sh
# Requires: curl, git (optional, for repo detection in fetch script).

set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXAMPLE="${HERE}/example-crate"
FETCH="${EXAMPLE}/scripts/fetch-obfuscator-plugin.sh"

if [[ ! -x "${FETCH}" ]]; then
  chmod +x "${FETCH}"
fi

"${FETCH}" "${1:-latest}"

echo ""
echo "Next: cd \"${EXAMPLE}\" && cargo build --release"
echo "  (rust-toolchain.toml pins nightly; rustup installs it on first cargo.)"
