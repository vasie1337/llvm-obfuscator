# Download the standalone pass plugin from GitHub Releases into vendor\obfuscator.dll
# (for LLVM opt on Windows — not for rustup rustc; see README).
#
# Usage:
#   .\scripts\fetch-obfuscator-plugin.ps1 [-Tag latest]
# Example:
#   .\scripts\fetch-obfuscator-plugin.ps1 -Tag v1.1.0
#
# Repository: $env:GITHUB_REPOSITORY ("owner/name") or inferred from git remote at repo root.

param(
    [string] $Tag = "latest"
)

$ErrorActionPreference = "Stop"

$ScriptDir = if ($PSScriptRoot) { $PSScriptRoot } else { Split-Path -Parent $MyInvocation.MyCommand.Path }
$CrateRoot = Split-Path -Parent $ScriptDir
$RepoRoot = (Resolve-Path (Join-Path $ScriptDir "..\..\..")).Path
$VendorDir = Join-Path $CrateRoot "vendor"
$OutFile = Join-Path $VendorDir "obfuscator.dll"
$AssetName = "obfuscator-x86_64-pc-windows-msvc.dll"

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

if ($Tag -eq "latest") {
    $Url = "https://github.com/$Repo/releases/latest/download/$AssetName"
} else {
    $Url = "https://github.com/$Repo/releases/download/$Tag/$AssetName"
}

New-Item -ItemType Directory -Force -Path $VendorDir | Out-Null
Write-Host "Downloading $Url"
Invoke-WebRequest -Uri $Url -OutFile $OutFile
Write-Host "Wrote $OutFile"
