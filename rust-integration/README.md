# Rust (`-Z llvm-plugins`)

The pass must match **rustc’s bundled LLVM**. This repo targets **LLVM 20** (e.g. **`nightly-2025-08-06`** — check with `rustc -vV | grep LLVM`).

**Plugin:** download **`obfuscator-x86_64-unknown-linux-gnu.so`** from Releases, or build the repo with **`-DOBFUSCATOR_STANDALONE_MODULE=ON`** (see root [README.md](../README.md)).

## Quick install (example crate)

From the **repository root**:

| Host | Command |
|------|---------|
| **Linux / WSL** | `bash rust-integration/setup.sh` |
| **Windows (PowerShell)** | `.\rust-integration\setup.ps1` (downloads `vendor/obfuscator.so`; run **cargo from WSL or Linux** so the plugin loads) |

Then:

```bash
cd rust-integration/example-crate
cargo build --release
```

`rust-toolchain.toml` pins the nightly; **rustup** installs it on the first `cargo` invocation.

**Cross-compiling** to Windows from Linux/WSL uses the same **`obfuscator.so`** with `--target x86_64-pc-windows-gnu` (MinGW). Only **your crate** is obfuscated; `std` is prebuilt.

**Native Windows `rustc`:** stock Windows rustup often **cannot** load LLVM plugins; prefer **WSL2** or Linux for Rust obfuscation. For **`opt` / clang** on Windows, build or download **`obfuscator-x86_64-pc-windows-msvc.dll`** (see root README), or run `.\rust-integration\setup.ps1 -OptDll` to fetch it beside the `.so`.

---

## Manual `RUSTFLAGS`

```bash
RUSTFLAGS="-Z llvm-plugins=/absolute/path/to/obfuscator.so" \
  cargo +nightly-2025-08-06 build --release
```

Copy **[cargo-config.toml](cargo-config.toml)** to your project as `.cargo/config.toml` and set the path. A working layout is **[example-crate/](example-crate/)**.
