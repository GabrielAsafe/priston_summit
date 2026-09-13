param(
    [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")),
    [string]$Python = "python",
    [string]$Database = "Database\summit.sqlite3",
    [string]$Backup = ""
)

$ErrorActionPreference = "Stop"
Set-Location $RepoRoot

if ([System.IO.Path]::IsPathRooted($Database)) { $DbPath = $Database } else { $DbPath = Join-Path $RepoRoot $Database }
if ([string]::IsNullOrWhiteSpace($Backup)) {
    $stamp = Get-Date -Format "yyyyMMdd_HHmmss"
    $Backup = Join-Path $RepoRoot "Database\backups\summit_preflight_$stamp.sqlite3"
} elseif (-not [System.IO.Path]::IsPathRooted($Backup)) {
    $Backup = Join-Path $RepoRoot $Backup
}

function Run-Python([string[]]$Args) {
    & $Python @Args
    if ($LASTEXITCODE -ne 0) { throw "Python command failed with exit code $LASTEXITCODE: $($Args -join ' ')" }
}

Write-Host "[1/4] Controlled validation suite"
Run-Python @("tools\run_controlled_validation.py", "--jobs", "6", "--output", "docs\WINDOWS_PREFLIGHT_VALIDATION.txt")

Write-Host "[2/4] SQLite status"
Run-Python @("tools\sqlite_local_admin.py", "--db", $DbPath, "status")

Write-Host "[3/4] Local-only readiness audit"
Run-Python @("tools\sqlite_local_admin.py", "--db", $DbPath, "audit-local-only", "--details")

Write-Host "[4/4] Consistent rollback backup"
Run-Python @("tools\sqlite_local_admin.py", "--db", $DbPath, "prepare-local-only", $Backup)

Write-Host ""
Write-Host "PRE-FLIGHT PASS"
Write-Host "Rollback backup: $Backup"
Write-Host "No runtime policy was changed. Keep legacy_billing_fallback=1 for the first Windows smoke run."
Write-Host "After the provider-compatible smoke passes, explicitly set fallback off, restart, and repeat the runtime checklist."
