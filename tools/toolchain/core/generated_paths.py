from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path


OUT_DIR_NAME = "out"
BUILD_DIR_NAME = "build"
TIDY_DIR_NAME = "tidy"
TEST_DIR_NAME = "test"
VALIDATE_DIR_NAME = "validate"
FULL_OUTPUT_LOG_NAME = "output.full.log"

_OUTPUT_APP_ALIASES = {
    "tracer_core": "tracer_core_shell",
}


@dataclass(frozen=True, slots=True)
class BuildLayout:
    app_name: str
    app_segment: str
    logical_build_dir: str
    root: Path
    bin_dir: Path


@dataclass(frozen=True, slots=True)
class TidyLayout:
    app_name: str
    app_segment: str
    workspace_name: str
    root: Path
    tasks_dir: Path
    tasks_done_dir: Path
    rename_dir: Path
    automation_dir: Path
    analysis_compile_db_dir: Path
    analysis_compile_db_path: Path
    tidy_result_path: Path
    batch_state_path: Path
    refresh_state_path: Path
    flow_state_path: Path


@dataclass(frozen=True, slots=True)
class TestResultLayout:
    result_target: str
    root: Path
    workspace_dir: Path
    artifacts_dir: Path
    logs_dir: Path
    quality_gates_dir: Path
    result_json_path: Path
    result_cases_json_path: Path
    output_log_path: Path
    output_full_log_path: Path
    runtime_guard_root: Path


@dataclass(frozen=True, slots=True)
class ValidationLayout:
    run_name: str
    root: Path
    logs_dir: Path
    tracks_dir: Path
    summary_json_path: Path
    output_log_path: Path
    output_full_log_path: Path


def normalize_output_app_name(app_name: str) -> str:
    normalized = (app_name or "").strip()
    if not normalized:
        raise ValueError("empty app_name")
    return _OUTPUT_APP_ALIASES.get(normalized, normalized)


def resolve_out_root(repo_root: Path) -> Path:
    return (repo_root / OUT_DIR_NAME).resolve()


def resolve_build_root(repo_root: Path) -> Path:
    return resolve_out_root(repo_root) / BUILD_DIR_NAME


def resolve_tidy_root(repo_root: Path) -> Path:
    return resolve_out_root(repo_root) / TIDY_DIR_NAME


def resolve_test_root(repo_root: Path) -> Path:
    return resolve_out_root(repo_root) / TEST_DIR_NAME


def resolve_validate_root(repo_root: Path) -> Path:
    return resolve_out_root(repo_root) / VALIDATE_DIR_NAME


def resolve_build_layout(
    repo_root: Path,
    app_name: str,
    logical_build_dir: str,
) -> BuildLayout:
    app_segment = normalize_output_app_name(app_name)
    normalized_build_dir = (logical_build_dir or "").strip()
    if not normalized_build_dir:
        raise ValueError("empty logical_build_dir")
    root = (resolve_build_root(repo_root) / app_segment / normalized_build_dir).resolve()
    return BuildLayout(
        app_name=app_name,
        app_segment=app_segment,
        logical_build_dir=normalized_build_dir,
        root=root,
        bin_dir=(root / "bin").resolve(),
    )


def resolve_tidy_layout(
    repo_root: Path,
    app_name: str,
    workspace_name: str,
) -> TidyLayout:
    app_segment = normalize_output_app_name(app_name)
    normalized_workspace = (workspace_name or "").strip()
    if not normalized_workspace:
        raise ValueError("empty workspace_name")
    root = (resolve_tidy_root(repo_root) / app_segment / normalized_workspace).resolve()
    analysis_compile_db_dir = (root / "analysis_compile_db").resolve()
    return TidyLayout(
        app_name=app_name,
        app_segment=app_segment,
        workspace_name=normalized_workspace,
        root=root,
        tasks_dir=(root / "tasks").resolve(),
        tasks_done_dir=(root / "tasks_done").resolve(),
        rename_dir=(root / "rename").resolve(),
        automation_dir=(root / "automation").resolve(),
        analysis_compile_db_dir=analysis_compile_db_dir,
        analysis_compile_db_path=(analysis_compile_db_dir / "compile_commands.json").resolve(),
        tidy_result_path=(root / "tidy_result.json").resolve(),
        batch_state_path=(root / "batch_state.json").resolve(),
        refresh_state_path=(root / "refresh_state.json").resolve(),
        flow_state_path=(root / "flow_state.json").resolve(),
    )


def resolve_test_result_layout(
    repo_root: Path,
    result_target: str,
) -> TestResultLayout:
    normalized_target = (result_target or "").strip()
    if not normalized_target:
        raise ValueError("empty result_target")
    root = (resolve_test_root(repo_root) / normalized_target).resolve()
    logs_dir = (root / "logs").resolve()
    return TestResultLayout(
        result_target=normalized_target,
        root=root,
        workspace_dir=(root / "workspace").resolve(),
        artifacts_dir=(root / "artifacts").resolve(),
        logs_dir=logs_dir,
        quality_gates_dir=(root / "quality_gates").resolve(),
        result_json_path=(root / "result.json").resolve(),
        result_cases_json_path=(root / "result_cases.json").resolve(),
        output_log_path=(logs_dir / "output.log").resolve(),
        output_full_log_path=(logs_dir / FULL_OUTPUT_LOG_NAME).resolve(),
        runtime_guard_root=(root / "runtime_guard").resolve(),
    )


def resolve_validation_layout(repo_root: Path, run_name: str) -> ValidationLayout:
    normalized_run_name = (run_name or "").strip()
    if not normalized_run_name:
        raise ValueError("empty run_name")
    root = (resolve_validate_root(repo_root) / normalized_run_name).resolve()
    logs_dir = (root / "logs").resolve()
    return ValidationLayout(
        run_name=normalized_run_name,
        root=root,
        logs_dir=logs_dir,
        tracks_dir=(root / "tracks").resolve(),
        summary_json_path=(root / "summary.json").resolve(),
        output_log_path=(logs_dir / "output.log").resolve(),
        output_full_log_path=(logs_dir / FULL_OUTPUT_LOG_NAME).resolve(),
    )


def resolve_test_result_layout_for_app(repo_root: Path, app_name: str) -> TestResultLayout:
    from ..services.suite_registry import resolve_result_output_name

    return resolve_test_result_layout(repo_root, resolve_result_output_name(app_name))
