param()

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$roundsScript = Join-Path $scriptDir "run_tidy_all_unattended_rounds.ps1"

if (-not (Test-Path -Path $roundsScript -PathType Leaf)) {
    Write-Error "Missing script: $roundsScript"
    exit 1
}

& $roundsScript -Rounds 0
exit $LASTEXITCODE
