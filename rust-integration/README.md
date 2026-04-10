# Using the Obfuscator with Rust Projects

## Prerequisites

1. **Build the standalone pass** (does not link libLLVM — uses the host's LLVM at runtime):

```bash
cd /path/to/llvm-obfuscator
mkdir -p build-rustc && cd build-rustc
cmake -G Ninja \
    -DLLVM_DIR=/usr/lib/llvm-20/lib/cmake/llvm \
    -DOBFUSCATOR_STANDALONE_MODULE=ON \
    ..
ninja
```

This produces `build-rustc/passes/Obfuscator.so`.

2. **Install a nightly toolchain with matching LLVM 20**:

```bash
rustup install nightly-2025-08-06
```

> **Why this specific nightly?** The pass is built against LLVM 20 headers.
> `nightly-2025-08-06` is the latest nightly that ships LLVM 20.
> Newer nightlies use LLVM 21+, which is ABI-incompatible.

## Quick start

```bash
RUSTFLAGS="-Z llvm-plugins=/absolute/path/to/build-rustc/passes/Obfuscator.so" \
    cargo +nightly-2025-08-06 build --release
```

## Cargo config (recommended)

Copy `cargo-config.toml` into your Rust project as `.cargo/config.toml`,
then edit the path to `Obfuscator.so`:

```bash
mkdir -p /path/to/your-rust-project/.cargo
cp cargo-config.toml /path/to/your-rust-project/.cargo/config.toml
# Edit the path in the file
```

Then build with:

```bash
cargo +nightly-2025-08-06 build --release
```

## What gets obfuscated

All passes run automatically at optimization levels >= 1:

- **InstructionSubstitution** — replaces arithmetic with equivalent but harder-to-read forms
- **MBASubstitution** — mixed boolean-arithmetic substitution
- **ControlFlowFlattening** — converts structured control flow into a switch-based dispatcher
- **StringEncryption** — encrypts string literals, decrypted at runtime

Only **your crate's code** is obfuscated. The Rust standard library is pre-compiled
and not affected.

## Matching LLVM versions

The pass **must** be loaded into a rustc whose bundled LLVM matches the version
the pass was compiled against. To check:

```bash
rustup run nightly-2025-08-06 rustc -vV | grep LLVM
# Should show: LLVM version: 20.x.x
```

If you rebuild the pass against a different LLVM version, find the corresponding
nightly with:

```bash
rustup run nightly-YYYY-MM-DD rustc -vV | grep LLVM
```
