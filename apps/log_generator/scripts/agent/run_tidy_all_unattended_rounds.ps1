param(
    [int]$Rounds = 0,
    [int]$StableLimit = 3,
    [int]$SleepSeconds = 2,
    [string]$PromptFile = ""
)

$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $scriptDir "..\..\..\.."))

$appName = "log_generator"
$tasksDir = Join-Path $repoRoot "apps\$appName\build_tidy\tasks"
$resultJson = Join-Path $repoRoot "test\output\$appName\result.json"
$workflowDefault = Join-Path $repoRoot ".agent\workflows\lg-tidy-all.md"
if ([string]::IsNullOrWhiteSpace($PromptFile)) {
    $PromptFile = $workflowDefault
}

function Resolve-CursorAgentCommand {
    if (-not [string]::IsNullOrWhiteSpace($env:CURSOR_AGENT_CMD)) {
        return $env:CURSOR_AGENT_CMD
    }

    $cursorAgent = Get-Command "cursor-agent" -ErrorAction SilentlyContinue
    if ($null -ne $cursorAgent) {
        return "cursor-agent"
    }

    $cursor = Get-Command "cursor" -ErrorAction SilentlyContinue
    if ($null -ne $cursor) {
        return "cursor"
    }

    return "cursor-agent"
}

function Get-PendingCount {
    param([string]$Root)
    if (-not (Test-Path -Path $Root -PathType Container)) {
        return 0
    }
    return @(
        Get-ChildItem -Path $Root -Recurse -File -Filter "task_*.log" -ErrorAction SilentlyContinue
    ).Count
}

function Test-ResultSuccess {
    param([string]$ResultPath)
    if (-not (Test-Path -Path $ResultPath -PathType Leaf)) {
        return $false
    }
    try {
        $json = Get-Content -Path $ResultPath -Raw | ConvertFrom-Json
        return ($json.success -eq $true)
    } catch {
        return $false
    }
}

function Build-Prompt {
    param([string]$WorkflowPath, [string]$Repo)
    $workflowFull = [System.IO.Path]::GetFullPath($WorkflowPath)
    $repoFull = [System.IO.Path]::GetFullPath($Repo)
    $workflowRel = $workflowFull
    if ($workflowFull.StartsWith($repoFull, [System.StringComparison]::OrdinalIgnoreCase)) {
        $workflowRel = $workflowFull.Substring($repoFull.Length).TrimStart('\').Replace('\', '/')
    }

    return @"
Continue the repository workflow: `$workflowRel`.

Hard constraints:
1) Only finish when no `task_*.log` exists under `apps/log_generator/build_tidy/tasks/`.
2) Keep `test/output/log_generator/result.json` with `success: true`.
3) Partial progress is not completion; continue to next task immediately.
4) Follow keep-going policy while generating tidy tasks.

If blocked by compile/test failure, fix root cause first, then continue workflow.
"@
}

function Invoke-CursorAgentOnce {
    param([string]$CommandName, [string]$Prompt)
    $normalized = $CommandName.Replace('\', '/')
    $baseName = [System.IO.Path]::GetFileName($normalized).ToLowerInvariant()
    if ($baseName -in @("cursor", "cursor.exe", "cursor.cmd")) {
        & $CommandName agent -p $Prompt
    } else {
        & $CommandName -p $Prompt
    }
    return $LASTEXITCODE
}

if (-not (Test-Path -Path $PromptFile -PathType Leaf)) {
    Write-Error "Prompt/workflow file not found: $PromptFile"
    exit 1
}

$cursorCmd = Resolve-CursorAgentCommand
$cursorResolved = Get-Command $cursorCmd -ErrorAction SilentlyContinue
if ($null -eq $cursorResolved) {
    Write-Error "Cursor CLI command '$cursorCmd' not found in PATH. Tried: cursor-agent, cursor."
    exit 1
}

$prompt = Build-Prompt -WorkflowPath $PromptFile -Repo $repoRoot
$round = 0
$noProgressRounds = 0

Write-Host "--- Cursor unattended tidy-all runner started."
Write-Host "--- Repo: $repoRoot"
Write-Host "--- Workflow prompt source: $PromptFile"
Write-Host "--- Tasks dir: $tasksDir"
Write-Host "--- Result json: $resultJson"
Write-Host "--- Cursor command: $cursorCmd"
Write-Host "--- Round limit: $Rounds (0 means until completed)"

while ($true) {
    $beforePending = Get-PendingCount -Root $tasksDir

    if ($beforePending -eq 0) {
        if (Test-ResultSuccess -ResultPath $resultJson) {
            Write-Host "--- Completed: no pending tasks and result.json success=true."
            exit 0
        }
        Write-Host "--- No pending tasks, but result.json success!=true. Running one agent round to recover."
    }

    $round++
    if ($Rounds -gt 0 -and $round -gt $Rounds) {
        Write-Host "--- Reached round limit ($Rounds). Stop."
        exit 3
    }

    Write-Host "--- Round $round`: pending before=$beforePending"
    $agentExit = Invoke-CursorAgentOnce -CommandName $cursorCmd -Prompt $prompt
    if ($agentExit -ne 0) {
        Write-Host "--- Cursor agent exited with code $agentExit."
        exit $agentExit
    }

    $afterPending = Get-PendingCount -Root $tasksDir
    $resultOk = Test-ResultSuccess -ResultPath $resultJson
    Write-Host "--- Round $round`: pending after=$afterPending, result.success=$resultOk"

    if ($afterPending -eq 0 -and $resultOk) {
        Write-Host "--- Completed: all tasks cleared and test result valid."
        exit 0
    }

    if ($afterPending -lt $beforePending) {
        $noProgressRounds = 0
    } else {
        $noProgressRounds++
        Write-Host "--- No pending reduction this round (streak=$noProgressRounds)."
    }

    if ($noProgressRounds -ge $StableLimit) {
        Write-Host "--- No progress for $noProgressRounds rounds. Stop for manual inspection."
        exit 4
    }

    Start-Sleep -Seconds $SleepSeconds
}
