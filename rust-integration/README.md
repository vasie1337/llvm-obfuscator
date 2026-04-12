# Rust (`-Z llvm-plugins`)

The pass must match **rustc’s bundled LLVM**. This repo targets **LLVM 20** (e.g. **`nightly-2025-08-06`** — check with `rustc -vV | grep LLVM`).

---

## `cargo obfuscate` — zero config, any project

Install the subcommand once:

```bash
cargo install --git https://github.com/vasie1337/llvm-obfuscator cargo-obfuscate
```

Then in **any** Rust project, replace `cargo build` with:

```bash
cargo obfuscate build --release
# cross-compile to Windows from Linux/WSL:
cargo obfuscate build --target x86_64-pc-windows-gnu --release
```

That’s it — no `rust-toolchain.toml`, no `.cargo/config.toml`, no manual plugin download.

On first run `cargo-obfuscate`:
1. Installs `nightly-2025-08-06` via rustup (if not already present)
2. Downloads the plugin from GitHub Releases into `~/.cargo/obfuscate-plugin/` (cached for all future runs)
3. Passes `-Z llvm-plugins=<cached-path>` to rustc automatically

**Native Windows:** stock Windows rustup often cannot load LLVM plugins — prefer WSL2 or Linux.

---

## Manual setup (example crate)

**Plugin:** download **`obfuscator-x86_64-unknown-linux-gnu.so`** from Releases, or build the repo with **`-DOBFUSCATOR_STANDALONE_MODULE=ON`** (see root [README.md](../README.md)).

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

---

## Manual `RUSTFLAGS`

```bash
RUSTFLAGS="-Z llvm-plugins=/absolute/path/to/obfuscator.so" \
  cargo +nightly-2025-08-06 build --release
```

Copy **[cargo-config.toml](cargo-config.toml)** to your project as `.cargo/config.toml` and set the path. A working layout is **[example-crate/](example-crate/)**.
