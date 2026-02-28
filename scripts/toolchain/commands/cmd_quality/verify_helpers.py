import json
import os
import sys
from pathlib import Path

from ...services.suite_registry import resolve_suite_build_app


def resolve_suite_config_override(
    suite_name: str,
    profile_name: str | None,
) -> str | None:
    normalized_profile = (profile_name or "").strip().lower()
    if not normalized_profile:
        return None

    if suite_name == "tracer_android":
        if normalized_profile == "android_style":
            return "config_android_style.toml"
        if normalized_profile == "android_ci":
            return "config_android_ci.toml"
        if normalized_profile == "android_device":
            return "config_android_device.toml"
    return None


def write_build_only_result_json(
    repo_root: Path,
    app_name: str,
    build_dir_name: str,
    success: bool,
    exit_code: int,
    duration_seconds: float,
    error_message: str = "",
    build_only: bool = True,
) -> None:
    output_root = repo_root / "test" / "output" / app_name
    logs_dir = output_root / "logs"
    output_root.mkdir(parents=True, exist_ok=True)
    logs_dir.mkdir(parents=True, exist_ok=True)

    failed_cases: list[dict] = []
    if not success:
        failed_cases.append(
            {
                "module": "verify",
                "name": "build",
                "status": "FAIL",
                "log_file": "",
                "messages": [error_message] if error_message else [],
                "return_code": exit_code,
                "command": [],
                "stderr_tail": [error_message] if error_message else [],
                "stdout_tail": [],
            }
        )

    result_payload = {
        "success": success,
        "exit_code": exit_code,
        "total_tests": 1,
        "total_failed": 0 if success else 1,
        "duration_seconds": round(duration_seconds, 3),
        "log_dir": str(logs_dir.resolve()),
        "modules": [
            {
                "name": "build",
                "total": 1,
                "passed": 1 if success else 0,
                "failed": 0 if success else 1,
            }
        ],
        "failed_cases": failed_cases,
        "error_message": error_message if not success else "",
        "build_only": build_only,
        "build_dir": build_dir_name,
    }
    result_cases_payload = {
        "success": success,
        "total_cases": 1,
        "total_failed": 0 if success else 1,
        "cases": failed_cases,
    }

    (output_root / "result.json").write_text(
        json.dumps(result_payload, indent=2, ensure_ascii=False),
        encoding="utf-8",
    )
    (output_root / "result_cases.json").write_text(
        json.dumps(result_cases_payload, indent=2, ensure_ascii=False),
        encoding="utf-8",
    )


def run_native_core_runtime_tests(
    repo_root: Path,
    setup_env_fn,
    run_command_fn,
    app_name: str,
    build_dir_name: str,
) -> int:
    build_app = resolve_suite_build_app(app_name) or app_name
    if build_app != "tracer_windows_cli":
        return 0

    app_root = (
        repo_root / "apps" / "tracer_cli" / "windows"
        if build_app == "tracer_windows_cli"
        else repo_root / "apps" / build_app
    )
    bin_dir = app_root / build_dir_name / "bin"
    suffix = ".exe" if os.name == "nt" else ""
    tests = [
        "tracer_core_c_api_smoke_tests",
        "tracer_core_c_api_stability_tests",
    ]

    for test_name in tests:
        executable = bin_dir / f"{test_name}{suffix}"
        if not executable.exists():
            print(
                f"--- verify: skip native core runtime test `{test_name}` "
                f"(missing executable: {executable})"
            )
            continue
        print(f"--- verify: running native core runtime test `{test_name}`")
        ret = run_command_fn(
            [str(executable)],
            cwd=bin_dir,
            env=setup_env_fn(),
        )
        if ret != 0:
            return ret
    return 0


def run_report_markdown_gates(
    repo_root: Path,
    setup_env_fn,
    run_command_fn,
    app_name: str,
    build_dir_name: str,
) -> int:
    build_app = resolve_suite_build_app(app_name) or app_name
    if build_app != "tracer_windows_cli":
        return 0

    app_root = (
        repo_root / "apps" / "tracer_cli" / "windows"
        if build_app == "tracer_windows_cli"
        else repo_root / "apps" / build_app
    )
    cli_bin = app_root / build_dir_name / "bin" / (
        "time_tracer_cli.exe" if os.name == "nt" else "time_tracer_cli"
    )
    export_root = (
        repo_root / "test" / "output" / "tracer_windows_cli" / "artifacts" / "reports" / "markdown"
    )
    db_path = (
        repo_root
        / "test"
        / "output"
        / "tracer_windows_cli"
        / "workspace"
        / "output"
        / "db"
        / "time_data.sqlite3"
    )
    current_cases_dir = (
        repo_root / "temp" / "report_markdown_cases" / "current_v1"
    )
    golden_dir = repo_root / "test" / "golden" / "report_markdown" / "v1"

    collect_cmd = [
        sys.executable,
        "scripts/tools/collect_report_markdown_cases.py",
        "--export-root",
        str(export_root),
        "--output-dir",
        str(current_cases_dir),
        "--cli-bin",
        str(cli_bin),
        "--db-path",
        str(db_path),
        "--strict-text-policy",
    ]
    collect_ret = run_command_fn(
        collect_cmd,
        cwd=repo_root,
        env=setup_env_fn(),
    )
    if collect_ret != 0:
        return collect_ret

    audit_cmd = [
        sys.executable,
        "scripts/tools/report_consistency_audit.py",
        "--left-dir",
        str(golden_dir),
        "--right-dir",
        str(current_cases_dir),
        "--pattern",
        "*.md",
        "--output",
        "temp/report-md-golden-byte-audit.md",
        "--fail-on-diff",
    ]
    audit_ret = run_command_fn(
        audit_cmd,
        cwd=repo_root,
        env=setup_env_fn(),
    )
    if audit_ret != 0:
        return audit_ret

    render_cmd = [
        sys.executable,
        "scripts/tools/report_markdown_render_snapshot_check.py",
        "--left-dir",
        str(golden_dir),
        "--right-dir",
        str(current_cases_dir),
        "--pattern",
        "*.md",
        "--output",
        "temp/report-md-golden-render-check.json",
        "--fail-on-structure-diff",
    ]
    return run_command_fn(
        render_cmd,
        cwd=repo_root,
        env=setup_env_fn(),
    )


def run_internal_logic_tests(
    repo_root: Path,
    setup_env_fn,
    run_command_fn,
) -> int:
    test_cmd = [
        sys.executable,
        "-m",
        "unittest",
        "scripts.tests.test_verify_command",
        "scripts.tests.test_verify_cli_handler",
    ]
    print("--- verify: running internal logic tests (Python unit/component)")
    return run_command_fn(
        test_cmd,
        cwd=repo_root,
        env=setup_env_fn(),
    )
