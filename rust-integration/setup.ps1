# Download release assets into example-crate\vendor (same layout as setup.sh).
# Uses curl.exe (Windows 10+) — no Git Bash required for the Linux plugin file.
#
# Rust -Z llvm-plugins: run cargo from WSL or Linux so the .so loads; the project can live on NTFS.
# Optional -OptDll: also fetch obfuscator-x86_64-pc-windows-msvc.dll for opt/clang --load-pass-plugin.
#
# Usage:  .\rust-integration\setup.ps1 [-Tag latest] [-OptDll]

param(
    [string] $Tag = "latest",
    [switch] $OptDll
)

$ErrorActionPreference = "Stop"

$Here = if ($PSScriptRoot) { $PSScriptRoot } else { Split-Path -Parent $MyInvocation.MyCommand.Path }
$CrateRoot = Join-Path $Here "example-crate"
$VendorDir = Join-Path $CrateRoot "vendor"
$RepoRoot = (Resolve-Path (Join-Path $Here "..")).Path

function Get-GitHubRepo {
    if ($env:GITHUB_REPOSITORY) {
        return $env:GITHUB_REPOSITORY.Trim()
    }
    Push-Location $RepoRoot
    try {
        $url = git remote get-url origin 2>$null
        if ($LASTEXITCODE -ne 0 -or -not $url) { return $null }
        if ($url -match "github\.com[:/]([^/]+/[^/.]+)(\.git)?$") {
            return $Matches[1]
        }
    } finally {
        Pop-Location
    }
    return $null
}

$Repo = Get-GitHubRepo
if (-not $Repo) {
    Write-Error "Could not determine GitHub repo. Set GITHUB_REPOSITORY=owner/name and retry."
}

function Get-ReleaseUrl($AssetName) {
    if ($Tag -eq "latest") {
        return "https://github.com/$Repo/releases/latest/download/$AssetName"
    }
    return "https://github.com/$Repo/releases/download/$Tag/$AssetName"
}

New-Item -ItemType Directory -Force -Path $VendorDir | Out-Null

$SoUrl = Get-ReleaseUrl "obfuscator-x86_64-unknown-linux-gnu.so"
$SoOut = Join-Path $VendorDir "obfuscator.so"
Write-Host "Downloading $SoUrl"
& curl.exe -fsSL -o $SoOut $SoUrl
Write-Host "Wrote $SoOut"

if ($OptDll) {
    $DllUrl = Get-ReleaseUrl "obfuscator-x86_64-pc-windows-msvc.dll"
    $DllOut = Join-Path $VendorDir "obfuscator.dll"
    Write-Host "Downloading $DllUrl"
    & curl.exe -fsSL -o $DllOut $DllUrl
    Write-Host "Wrote $DllOut"
}

Write-Host ""
Write-Host "Next (WSL or Linux): cd `"$CrateRoot`" && cargo build --release"
