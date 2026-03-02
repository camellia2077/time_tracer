param(
    [ValidateSet("check", "fix")]
    [string]$Mode = "check",
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$FormatArgs = @()
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = (Resolve-Path (Join-Path $scriptDir "../../../../..")).Path
$runPy = Join-Path $repoRoot "scripts/run.py"

if (-not (Test-Path $runPy)) {
    Write-Error "scripts/run.py not found: $runPy"
}

$pythonCmd = if (Get-Command python -ErrorAction SilentlyContinue) { "python" } elseif (Get-Command py -ErrorAction SilentlyContinue) { "py" } else { "" }
if (-not $pythonCmd) {
    throw "python runtime not found (python/py)."
}

$cmd = @("scripts/run.py", "format", "--app", "tracer_windows_rust_cli", "--")
if ($Mode -eq "check") {
    $cmd += "--check"
}
$cmd += $FormatArgs

Write-Host "[rust fmt] mode=$Mode app=tracer_windows_rust_cli"
if ($pythonCmd -eq "py") {
    & py -3 @cmd
} else {
    & python @cmd
}
exit $LASTEXITCODE
