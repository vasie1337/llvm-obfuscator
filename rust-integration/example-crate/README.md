# Example crate

Fetches a **Linux** standalone plugin into `vendor/obfuscator.so` and builds with **`rust-toolchain.toml`** (LLVM 20 nightly).

```bash
chmod +x scripts/fetch-obfuscator-plugin.sh
./scripts/fetch-obfuscator-plugin.sh latest
cargo build --release
```

Optional: Windows **`.exe`** from Linux/WSL — `sudo apt install mingw-w64`, `rustup target add x86_64-pc-windows-gnu`, then `cargo build --release --target x86_64-pc-windows-gnu`.

Quick check: `strings target/release/obfuscator-example` should not show the hello string as plain ASCII if string encryption ran.

See **[../README.md](../README.md)** to build the plugin locally instead of downloading.
