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

### Standalone module (Rust / rustc)

For a **standalone** `.so` that does not link `libLLVM` (for use with hosts that ship their own LLVM, such as `rustc`), enable `OBFUSCATOR_STANDALONE_MODULE`. See **[rust-integration/README.md](rust-integration/README.md)** for prerequisites, matching LLVM versions, and `cargo` usage.

**Prebuilt Linux x86_64 plugin** assets are attached to **GitHub Releases** (`obfuscator-x86_64-unknown-linux-gnu.so`). A minimal template that uses them is **[rust-integration/example-crate/](rust-integration/example-crate/)**.

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
