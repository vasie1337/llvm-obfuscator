# Rust (`-Z llvm-plugins`)

The pass must match **rustc’s bundled LLVM**. This repo targets **LLVM 20** (e.g. **`nightly-2025-08-06`** — check with `rustc -vV | grep LLVM`).

**Plugin:** download **`obfuscator-x86_64-unknown-linux-gnu.so`** from Releases, or build the repo with **`-DOBFUSCATOR_STANDALONE_MODULE=ON`** (see root [README.md](../README.md)).

**Build:**

```bash
RUSTFLAGS="-Z llvm-plugins=/absolute/path/to/obfuscator.so" \
  cargo +nightly-2025-08-06 build --release
```

Copy **[cargo-config.toml](cargo-config.toml)** to your project as `.cargo/config.toml` and set the path. A working layout is **[example-crate/](example-crate/)**.

**Host:** Linux x86_64 or WSL. Cross-compiling to Windows from Linux uses the same **`obfuscator.so`** with `--target x86_64-pc-windows-gnu` (MinGW). Only **your crate** is obfuscated; `std` is prebuilt.
