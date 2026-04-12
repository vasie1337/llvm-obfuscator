use std::{env, fs, path::PathBuf, process};

const NIGHTLY: &str = "nightly-2025-08-06";
const REPO: &str = "vasie1337/llvm-obfuscator";

struct Platform {
    asset_name: &'static str,
    local_name: &'static str,
}

fn platform() -> Platform {
    match env::consts::OS {
        "linux" => Platform {
            asset_name: "obfuscator-x86_64-unknown-linux-gnu.so",
            local_name: "obfuscator.so",
        },
        "windows" => Platform {
            asset_name: "obfuscator-x86_64-pc-windows-msvc.dll",
            local_name: "obfuscator.dll",
        },
        os => {
            eprintln!("[cargo-obfuscate] Unsupported OS: {os}");
            process::exit(1);
        }
    }
}

fn cache_dir() -> PathBuf {
    let base = env::var("CARGO_HOME")
        .map(PathBuf::from)
        .unwrap_or_else(|_| {
            let home = env::var("HOME")
                .or_else(|_| env::var("USERPROFILE"))
                .expect("HOME or USERPROFILE must be set");
            PathBuf::from(home).join(".cargo")
        });
    base.join("obfuscate-plugin")
}

fn ensure_plugin() -> PathBuf {
    let plat = platform();
    let dir = cache_dir();
    let path = dir.join(plat.local_name);
    if path.exists() {
        return path;
    }
    fs::create_dir_all(&dir).expect("failed to create plugin cache dir");
    let url = format!(
        "https://github.com/{REPO}/releases/latest/download/{}",
        plat.asset_name
    );
    eprintln!("[cargo-obfuscate] Downloading plugin from GitHub Releases...");
    eprintln!("[cargo-obfuscate]   {url}");

    let curl = if cfg!(windows) { "curl.exe" } else { "curl" };
    let status = process::Command::new(curl)
        .args(["-fsSL", "--output", path.to_str().unwrap(), &url])
        .status()
        .unwrap_or_else(|e| {
            eprintln!("[cargo-obfuscate] Failed to run {curl}: {e}");
            eprintln!("[cargo-obfuscate] Download manually: {url}");
            eprintln!("[cargo-obfuscate] Save to: {}", path.display());
            process::exit(1);
        });

    if !status.success() {
        // Clean up partial download
        let _ = fs::remove_file(&path);
        eprintln!("[cargo-obfuscate] Download failed.");
        eprintln!("[cargo-obfuscate] Download manually: {url}");
        eprintln!("[cargo-obfuscate] Save to: {}", path.display());
        process::exit(1);
    }

    eprintln!("[cargo-obfuscate] Plugin cached at {}", path.display());
    path
}

fn ensure_toolchain() {
    let output = process::Command::new("rustup")
        .args(["toolchain", "list"])
        .output()
        .unwrap_or_else(|e| {
            eprintln!("[cargo-obfuscate] rustup not found: {e}");
            eprintln!("[cargo-obfuscate] Install rustup from https://rustup.rs");
            process::exit(1);
        });

    let list = String::from_utf8_lossy(&output.stdout);
    if !list.contains(NIGHTLY) {
        eprintln!("[cargo-obfuscate] Installing {NIGHTLY} (first run only)...");
        let status = process::Command::new("rustup")
            .args(["toolchain", "install", NIGHTLY, "--profile", "minimal"])
            .status()
            .unwrap_or_else(|e| {
                eprintln!("[cargo-obfuscate] rustup failed: {e}");
                process::exit(1);
            });
        if !status.success() {
            eprintln!("[cargo-obfuscate] Failed to install {NIGHTLY}");
            process::exit(1);
        }
    }
}

fn main() {
    let args: Vec<String> = env::args().collect();

    // When invoked as a cargo subcommand:
    //   user types:  cargo obfuscate build --release
    //   cargo calls: cargo-obfuscate obfuscate build --release
    // args[0] = binary, args[1] = "obfuscate", args[2..] = actual cargo args
    let cargo_args: Vec<String> = match args.get(1).map(String::as_str) {
        Some("obfuscate") => args[2..].to_vec(),
        _ => args[1..].to_vec(),
    };

    if cargo_args.is_empty() {
        eprintln!("Usage: cargo obfuscate <cargo-command> [args...]");
        eprintln!("Examples:");
        eprintln!("  cargo obfuscate build --release");
        eprintln!("  cargo obfuscate build --target x86_64-pc-windows-gnu --release");
        process::exit(1);
    }

    if cfg!(windows) {
        eprintln!("[cargo-obfuscate] Note: native Windows rustc may not support LLVM plugins.");
        eprintln!("[cargo-obfuscate] If the build fails to load the plugin, use WSL2.");
    }

    ensure_toolchain();
    let plugin_path = ensure_plugin();

    let plugin_flag = format!(
        "-Z llvm-plugins={}",
        plugin_path.to_str().expect("plugin path must be valid UTF-8")
    );
    // --emit=llvm-ir must be present alongside the default dep-info,link emit
    // because rustc only fires module-level LLVM extension points (needed for
    // StringEncryption) when the IR-emission path is active.  The .ll files
    // land in target/*/deps/ and are harmless (target/ is gitignored).
    let extra = "--emit=llvm-ir";
    let base_flags = format!("{plugin_flag} {extra}");
    let rustflags = match env::var("RUSTFLAGS").ok().filter(|s| !s.is_empty()) {
        Some(existing) => format!("{existing} {base_flags}"),
        None => base_flags,
    };

    let mut cmd_args = vec![format!("+{NIGHTLY}")];
    cmd_args.extend(cargo_args);

    let status = process::Command::new("cargo")
        .args(&cmd_args)
        .env("RUSTFLAGS", rustflags)
        .status()
        .unwrap_or_else(|e| {
            eprintln!("[cargo-obfuscate] Failed to run cargo: {e}");
            process::exit(1);
        });

    process::exit(status.code().unwrap_or(1));
}
