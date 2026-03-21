import argparse
import re
import shutil
import subprocess
import time
from pathlib import Path

from tools.toolchain.core.generated_paths import resolve_test_result_layout

from .runtime_guard_bundle import (
    DEFAULT_CLI_EXE,
    copy_runtime_bundle,
    ensure_source_runtime_ready,
    load_runtime_bundle_spec,
    resolve_runtime_library_names,
    resolve_source_bin_dir,
)
from .runtime_guard_scenarios import RuntimeGuardScenario, build_scenarios

ANSI_ESCAPE_RE = re.compile(r"\x1B\[[0-?]*[ -/]*[@-~]")


def _strip_ansi(text: str) -> str:
    return ANSI_ESCAPE_RE.sub("", text)


def _clean_workspace_dir(workspace_root: Path) -> None:
    if not workspace_root.exists():
        return

    last_error: OSError | None = None
    for _ in range(8):
        try:
            shutil.rmtree(workspace_root)
            return
        except OSError as error:
            last_error = error
            time.sleep(0.2)

    raise RuntimeError(f"failed to clean runtime-guard workspace: {workspace_root} ({last_error})")


def _run_scenario(
    source_bin: Path,
    workspace_root: Path,
    files_to_copy: list[str],
    folders_to_copy: list[str],
    scenario: RuntimeGuardScenario,
) -> tuple[bool, str]:
    scenario_bin = workspace_root / scenario.name / "bin"
    copy_runtime_bundle(source_bin, scenario_bin, files_to_copy, folders_to_copy)
    scenario.mutate(scenario_bin)

    cli_path = scenario_bin / DEFAULT_CLI_EXE
    if not cli_path.exists():
        return False, f"CLI executable not found in scenario bundle: {cli_path}"

    completed = subprocess.run(
        [str(cli_path), "query", "tree"],
        cwd=str(scenario_bin),
        capture_output=True,
        text=True,
        timeout=30,
        check=False,
    )

    merged_output = _strip_ansi((completed.stdout or "") + "\n" + (completed.stderr or "")).lower()
    actual_exit = completed.returncode

    if scenario.expect_success:
        if actual_exit not in scenario.expected_exits:
            return (
                False,
                f"expected success(exit in {scenario.expected_exits}), got exit={actual_exit}\n{merged_output.strip()}",
            )
    else:
        if actual_exit == 0:
            return False, "expected failure(exit!=0), got exit=0"
        if actual_exit not in scenario.expected_exits:
            return (
                False,
                f"expected failure(exit in {scenario.expected_exits}), got exit={actual_exit}\n{merged_output.strip()}",
            )

    for token in scenario.expected_tokens:
        if token.lower() not in merged_output:
            return (
                False,
                "missing expected token: " + token + "\n" + merged_output.strip(),
            )

    for token in scenario.unexpected_tokens:
        if token.lower() in merged_output:
            return (
                False,
                "found unexpected token: " + token + "\n" + merged_output.strip(),
            )

    return True, f"exit={actual_exit}"


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Run reusable runtime guard scenarios for tracer_windows_rust_cli."
    )
    parser.add_argument(
        "--build-dir",
        default=None,
        help="Build dir under apps/tracer_cli/windows (e.g. build_fast).",
    )
    parser.add_argument(
        "--bin-dir",
        default=None,
        help="Direct bin directory override (highest priority).",
    )
    parser.add_argument(
        "--workspace",
        default=None,
        help=("Workspace for guard cases. Default: out/test/artifact_windows_cli/runtime_guard"),
    )
    parser.add_argument(
        "--keep-workspace",
        action="store_true",
        help="Keep existing workspace content instead of cleaning before run.",
    )
    return parser.parse_args(argv)


def main(argv: list[str], repo_root: Path) -> int:
    args = parse_args(argv)

    try:
        source_bin = resolve_source_bin_dir(repo_root, args)
        ensure_source_runtime_ready(source_bin)
    except Exception as error:
        print(f"[runtime-guard] setup failed: {error}", flush=True)
        return 2

    workspace_root = (
        Path(args.workspace).resolve()
        if args.workspace
        else resolve_test_result_layout(repo_root, "artifact_windows_cli").runtime_guard_root
    )
    if workspace_root.exists() and not args.keep_workspace:
        try:
            _clean_workspace_dir(workspace_root)
        except Exception as error:
            print(f"[runtime-guard] setup failed: {error}", flush=True)
            return 2
    workspace_root.mkdir(parents=True, exist_ok=True)

    files_to_copy, folders_to_copy = load_runtime_bundle_spec(repo_root)
    core_dll_name, reports_shared_dll_name = resolve_runtime_library_names(source_bin)
    scenarios = build_scenarios(
        core_dll_name=core_dll_name,
        reports_shared_dll_name=reports_shared_dll_name,
    )
    failures: list[str] = []

    print(f"[runtime-guard] source bin: {source_bin}", flush=True)
    print(f"[runtime-guard] workspace : {workspace_root}", flush=True)
    print(f"[runtime-guard] scenarios : {len(scenarios)}", flush=True)

    for index, scenario in enumerate(scenarios, start=1):
        print(
            f"\n[{index}/{len(scenarios)}] {scenario.name} - {scenario.description}",
            flush=True,
        )
        ok, detail = _run_scenario(
            source_bin=source_bin,
            workspace_root=workspace_root,
            files_to_copy=files_to_copy,
            folders_to_copy=folders_to_copy,
            scenario=scenario,
        )
        status = "PASS" if ok else "FAIL"
        print(f"  -> {status}: {detail}", flush=True)
        if not ok:
            failures.append(f"{scenario.name}: {detail}")

    if failures:
        print("\n[runtime-guard] FAILED", flush=True)
        for item in failures:
            print(f" - {item}", flush=True)
        return 1

    print("\n[runtime-guard] PASSED", flush=True)
    return 0
