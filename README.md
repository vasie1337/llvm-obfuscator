# llvm-obfuscator

LLVM **New Pass Manager** plugin with obfuscation transforms (instruction substitution, MBA, bogus control flow, control-flow flattening, string encryption, and others). Built against **LLVM 20**.

**IDA demo (Hex-Rays):** [clean](files/clean.md) · [obfuscated](files/obfuscated.md)  

## Requirements

- CMake ≥ 3.20, Ninja (or another generator)
- LLVM 20 development packages (e.g. `llvm-20-dev`, `clang-20`, `opt-20` on Debian/Ubuntu)
- Adjust `LLVM_DIR` below if your install path differs

## Build

```bash
mkdir -p build && cd build
cmake -G Ninja -DLLVM_DIR=/usr/lib/llvm-20/lib/cmake/llvm ..
ninja
```

This produces `build/passes/obfuscator.so`, a loadable pass plugin for `opt` / `clang`.

### Standalone module (Rust)

For a **standalone** `.so` that does not link `libLLVM` (for **`-Z llvm-plugins`** with `rustc`), add `-DOBFUSCATOR_STANDALONE_MODULE=ON` to the CMake line above. Prebuilt **`obfuscator-x86_64-unknown-linux-gnu.so`** is on **GitHub Releases**. See **[rust-integration/README.md](rust-integration/README.md)** and **[rust-integration/example-crate/](rust-integration/example-crate/)**.

**Quick setup:** from the repo root run **`bash rust-integration/setup.sh`** (Linux/WSL) or **`.\rust-integration\setup.ps1`** on Windows (downloads the plugin; run **cargo from WSL or Linux**).

Rust obfuscation needs **Linux x86_64 or WSL** and a **nightly whose LLVM matches** the plugin (LLVM 20 today). Stock **Windows rustup** usually cannot load the plugin; build obfuscated Rust from Linux/WSL.

**Windows DLL for `opt`:** same CMake with MSVC, `-DOBFUSCATOR_STANDALONE_MODULE=ON`; output is `obfuscator.dll` under `passes/` (or `passes/Release/`). Example: `opt --load-pass-plugin=obfuscator.dll --passes=instsub -S in.ll -o out.ll`. Prebuilt **`obfuscator-x86_64-pc-windows-msvc.dll`** may be on Releases.

## Tests

From the repo root after building:

```bash
./tests/run_tests.sh
```

Or use CMake’s test runner:

```bash
cd build && ctest
```

## Obfuscate a C file

With the default build layout:

```bash
./scripts/obfuscate.sh path/to/source.c
```

Use `-p` to pick passes (comma-separated); run `./scripts/obfuscate.sh -h` for options.
