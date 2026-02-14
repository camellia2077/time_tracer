param(
    [switch]$RunWindowsChecks,
    [switch]$RunAndroidChecks,
    [string]$AndroidProjectDir = ""
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Fail([string]$Message) {
    Write-Error $Message
    exit 1
}

function Run-Command([string]$Command, [string]$WorkingDirectory) {
    Write-Host ">> $Command"
    Push-Location $WorkingDirectory
    try {
        Invoke-Expression $Command
        if ($LASTEXITCODE -ne 0) {
            Fail "Command failed (exit=$LASTEXITCODE): $Command"
        }
    }
    finally {
        Pop-Location
    }
}

$repoRoot = Resolve-Path "."

$contractPath = Join-Path $repoRoot "docs/time_tracer/platform_sync_contract.md"
$windowsHeaderPath = Join-Path $repoRoot "apps/time_tracer/src/infrastructure/platform/windows/windows_platform_clock.hpp"
$windowsSourcePath = Join-Path $repoRoot "apps/time_tracer/src/infrastructure/platform/windows/windows_platform_clock.cpp"
$androidHeaderPath = Join-Path $repoRoot "apps/time_tracer/src/infrastructure/platform/android/android_platform_clock.hpp"
$androidSourcePath = Join-Path $repoRoot "apps/time_tracer/src/infrastructure/platform/android/android_platform_clock.cpp"

if (-not (Test-Path $contractPath)) {
    Fail "Missing platform sync contract: $contractPath"
}
if (-not (Test-Path $windowsHeaderPath)) {
    Fail "Missing Windows header implementation: $windowsHeaderPath"
}
if (-not (Test-Path $windowsSourcePath)) {
    Fail "Missing Windows source implementation: $windowsSourcePath"
}
if (-not (Test-Path $androidHeaderPath)) {
    Fail "Missing Android header implementation: $androidHeaderPath"
}
if (-not (Test-Path $androidSourcePath)) {
    Fail "Missing Android source implementation: $androidSourcePath"
}

$contractContent = Get-Content -Path $contractPath -Raw
if ($contractContent -notmatch "Windows Status" -or
    $contractContent -notmatch "Android Status") {
    Fail "Contract table must contain Windows Status and Android Status columns."
}

$rowMatch = [regex]::Match(
    $contractContent,
    '(?m)^\| `IPlatformClock` \|.*$'
)
if (-not $rowMatch.Success) {
    Fail "Contract table is missing IPlatformClock row."
}

$cells = $rowMatch.Value.Trim("|").Split("|") | ForEach-Object { $_.Trim() }
if ($cells.Count -lt 6) {
    Fail "IPlatformClock table row is malformed."
}
if ([string]::IsNullOrWhiteSpace($cells[4])) {
    Fail "IPlatformClock Windows Status must not be empty."
}
if ([string]::IsNullOrWhiteSpace($cells[5])) {
    Fail "IPlatformClock Android Status must not be empty."
}

if ($RunWindowsChecks) {
    Run-Command "python scripts/run.py build --app time_tracer" $repoRoot
    Run-Command "ctest --test-dir apps/time_tracer/build_fast --output-on-failure" $repoRoot
    Run-Command "python test/run.py --suite time_tracer --agent --build-dir build_fast --concise" $repoRoot
}

if ($RunAndroidChecks) {
    $resolvedAndroidDir = ""
    if ([string]::IsNullOrWhiteSpace($AndroidProjectDir)) {
        $resolvedAndroidDir = $repoRoot
    } else {
        $resolvedAndroidDir = Resolve-Path $AndroidProjectDir
    }

    $gradleBat = Join-Path $resolvedAndroidDir "gradlew.bat"
    $gradleSh = Join-Path $resolvedAndroidDir "gradlew"
    if (Test-Path $gradleBat) {
        Run-Command ".\gradlew.bat :app:assembleDebug" $resolvedAndroidDir
    } elseif (Test-Path $gradleSh) {
        Run-Command "./gradlew :app:assembleDebug" $resolvedAndroidDir
    } else {
        Fail "Android checks requested but no gradle wrapper found in: $resolvedAndroidDir"
    }
}

Write-Host "[OK] Platform sync checks passed."
Write-Host "Usage examples:"
Write-Host "  pwsh ./scripts/check_platform_sync.ps1"
Write-Host "  pwsh ./scripts/check_platform_sync.ps1 -RunWindowsChecks"
Write-Host "  pwsh ./scripts/check_platform_sync.ps1 -RunWindowsChecks -RunAndroidChecks -AndroidProjectDir C:\path\to\android-project"
