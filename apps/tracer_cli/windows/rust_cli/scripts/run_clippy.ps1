param(
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$LintArgs = @()
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = (Resolve-Path (Join-Path $scriptDir "../../../../..")).Path
$runPy = Join-Path $repoRoot "tools/run.py"

if (-not (Test-Path $runPy)) {
    Write-Error "tools/run.py not found: $runPy"
}

$pythonCmd = if (Get-Command python -ErrorAction SilentlyContinue) { "python" } elseif (Get-Command py -ErrorAction SilentlyContinue) { "py" } else { "" }
if (-not $pythonCmd) {
    throw "python runtime not found (python/py)."
}

Write-Host "[rust clippy] app=tracer_windows_rust_cli"
$cmd = @("tools/run.py", "lint", "--app", "tracer_windows_rust_cli", "--")
$cmd += $LintArgs
if ($pythonCmd -eq "py") {
    & py -3 @cmd
} else {
    & python @cmd
}
exit $LASTEXITCODE
