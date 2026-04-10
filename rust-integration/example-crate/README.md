# Example Rust crate (llvm-obfuscator + prebuilt plugin)

This directory is a minimal template for building Rust with the obfuscator loaded via `-Z llvm-plugins`, using a **prebuilt** standalone `obfuscator.so` from this repository’s **GitHub Releases** (Linux x86_64, GNU). The release asset is named `obfuscator-x86_64-unknown-linux-gnu.so`.

## Requirements

- **Linux x86_64** (glibc). The published artifact is named `obfuscator-x86_64-unknown-linux-gnu.so`.
- **Rust nightly** with **LLVM 20** — this repo pins `nightly-2025-08-06` in `rust-toolchain.toml` (see [../README.md](../README.md) for why).

## One-time setup: download the plugin

After the first **GitHub Release** in this repository (created by pushing a `v*` tag), `latest` works. Until then, use a **local standalone build** (see below) or a CI artifact.

From this directory:

```bash
chmod +x scripts/fetch-obfuscator-plugin.sh
./scripts/fetch-obfuscator-plugin.sh latest
```

Or pin a release tag:

```bash
./scripts/fetch-obfuscator-plugin.sh v0.1.0
```

The script writes `vendor/obfuscator.so`, which matches `.cargo/config.toml`.

If you are not in a git clone of this repo, set the repository explicitly:

```bash
export GITHUB_REPOSITORY=your-org/llvm-obfuscator
./scripts/fetch-obfuscator-plugin.sh latest
```

## Build

```bash
cargo build --release
```

`rust-toolchain.toml` selects the pinned nightly; `cargo` does not need an explicit `+nightly-…` when run inside this crate.

## Alternative: build the plugin locally

If you do not use GitHub Releases, build the **standalone** module and copy it to `vendor/obfuscator.so`:

```bash
cd /path/to/llvm-obfuscator
mkdir -p build-rustc && cd build-rustc
cmake -G Ninja \
  -DLLVM_DIR=/usr/lib/llvm-20/lib/cmake/llvm \
  -DOBFUSCATOR_STANDALONE_MODULE=ON \
  ..
ninja
cp passes/obfuscator.so /path/to/rust-integration/example-crate/vendor/obfuscator.so
```

Then run `cargo build --release` as above.
