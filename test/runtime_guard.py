import argparse
import re
import shutil
import subprocess
import sys
from collections.abc import Callable
from dataclasses import dataclass
from pathlib import Path

try:
    import tomllib
except ImportError:  # pragma: no cover
    import tomli as tomllib  # type: ignore


ANSI_ESCAPE_RE = re.compile(r"\x1B\[[0-?]*[ -/]*[@-~]")

DEFAULT_APP_NAME = "tracer_windows_cli"
DEFAULT_CLI_EXE = "time_tracer_cli.exe"
DEFAULT_RUNTIME_FILES = [
    "time_tracer_cli.exe",
    "tracer_core.dll",
    "libreports_shared.dll",
    "libsqlite3-0.dll",
    "libtomlplusplus-3.dll",
    "libgcc_s_seh-1.dll",
    "libstdc++-6.dll",
    "libwinpthread-1.dll",
]
DEFAULT_RUNTIME_FOLDERS = ["config", "plugins"]


@dataclass(frozen=True)
class RuntimeGuardScenario:
    name: str
    description: str
    mutate: Callable[[Path], None]
    expect_success: bool
    expected_exit: int
    expected_tokens: list[str]
    unexpected_tokens: list[str]


def _strip_ansi(text: str) -> str:
    return ANSI_ESCAPE_RE.sub("", text)


def _auto_detect_build_dir(repo_root: Path, app_name: str) -> str | None:
    app_root = repo_root / "apps" / app_name
    for candidate in ("build_fast", "build_agent", "build_tidy", "build"):
        if (app_root / candidate / "bin").is_dir():
            return candidate
    return None


def _load_runtime_bundle_spec(repo_root: Path) -> tuple[list[str], list[str]]:
    env_path = repo_root / "test" / "suites" / "tracer_windows_cli" / "env.toml"
    if not env_path.exists():
        return DEFAULT_RUNTIME_FILES, DEFAULT_RUNTIME_FOLDERS

    try:
        with env_path.open("rb") as file:
            payload = tomllib.load(file)
    except Exception:
        return DEFAULT_RUNTIME_FILES, DEFAULT_RUNTIME_FOLDERS

    cleanup = payload.get("cleanup", {})
    if not isinstance(cleanup, dict):
        return DEFAULT_RUNTIME_FILES, DEFAULT_RUNTIME_FOLDERS

    files = cleanup.get("files_to_copy")
    folders = cleanup.get("folders_to_copy")

    selected_files = (
        [item for item in files if isinstance(item, str)] if isinstance(files, list) else []
    )
    selected_folders = (
        [item for item in folders if isinstance(item, str)] if isinstance(folders, list) else []
    )

    if not selected_files:
        selected_files = list(DEFAULT_RUNTIME_FILES)
    if not selected_folders:
        selected_folders = list(DEFAULT_RUNTIME_FOLDERS)
    return selected_files, selected_folders


def _copy_runtime_bundle(
    source_bin: Path,
    dest_bin: Path,
    files_to_copy: list[str],
    folders_to_copy: list[str],
) -> None:
    dest_bin.mkdir(parents=True, exist_ok=True)

    for file_rel in files_to_copy:
        src = source_bin / file_rel
        dst = dest_bin / file_rel
        if not src.exists():
            continue
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(src, dst)

    for folder_rel in folders_to_copy:
        src_dir = source_bin / folder_rel
        dst_dir = dest_bin / folder_rel
        if not src_dir.is_dir():
            continue
        shutil.copytree(src_dir, dst_dir, dirs_exist_ok=True)


def _ensure_source_runtime_ready(source_bin: Path) -> None:
    required = [
        source_bin / DEFAULT_CLI_EXE,
        source_bin / "tracer_core.dll",
        source_bin / "libreports_shared.dll",
        source_bin / "config" / "config.toml",
    ]
    missing = [str(path) for path in required if not path.exists()]
    if missing:
        raise RuntimeError(
            "Runtime guard source bundle is incomplete. Missing:\n- " + "\n- ".join(missing)
        )


def _resolve_source_bin_dir(repo_root: Path, args: argparse.Namespace) -> Path:
    if args.bin_dir:
        return Path(args.bin_dir).resolve()

    build_dir = args.build_dir
    if not build_dir:
        build_dir = _auto_detect_build_dir(repo_root, DEFAULT_APP_NAME)
    if not build_dir:
        raise RuntimeError("No build dir detected. Use --build-dir, --bin-dir, or build first.")
    return (repo_root / "apps" / DEFAULT_APP_NAME / build_dir / "bin").resolve()


def _mutate_nop(_: Path) -> None:
    return


def _remove_required(path: Path) -> None:
    if not path.exists():
        raise RuntimeError(f"Cannot mutate runtime guard case; file not found: {path}")
    path.unlink()


def _run_scenario(
    source_bin: Path,
    workspace_root: Path,
    files_to_copy: list[str],
    folders_to_copy: list[str],
    scenario: RuntimeGuardScenario,
) -> tuple[bool, str]:
    scenario_bin = workspace_root / scenario.name / "bin"
    _copy_runtime_bundle(source_bin, scenario_bin, files_to_copy, folders_to_copy)
    scenario.mutate(scenario_bin)

    cli_path = scenario_bin / DEFAULT_CLI_EXE
    if not cli_path.exists():
        return False, f"CLI executable not found in scenario bundle: {cli_path}"

    completed = subprocess.run(
        [str(cli_path), "tree"],
        cwd=str(scenario_bin),
        capture_output=True,
        text=True,
        timeout=30,
        check=False,
    )

    merged_output = _strip_ansi((completed.stdout or "") + "\n" + (completed.stderr or "")).lower()
    actual_exit = completed.returncode

    if scenario.expect_success:
        if actual_exit != scenario.expected_exit:
            return (
                False,
                f"expected success(exit={scenario.expected_exit}), got exit={actual_exit}\n{merged_output.strip()}",
            )
    else:
        if actual_exit == 0:
            return False, "expected failure(exit!=0), got exit=0"
        if actual_exit != scenario.expected_exit:
            return (
                False,
                f"expected failure(exit={scenario.expected_exit}), got exit={actual_exit}\n{merged_output.strip()}",
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


def _build_scenarios() -> list[RuntimeGuardScenario]:
    return [
        RuntimeGuardScenario(
            name="baseline_ok",
            description="完整 runtime bundle 可正常通过 bootstrap guard。",
            mutate=_mutate_nop,
            expect_success=True,
            expected_exit=0,
            expected_tokens=[],
            unexpected_tokens=[
                "runtime check failed",
                "configuration validation failed",
                "startup error",
            ],
        ),
        RuntimeGuardScenario(
            name="missing_core_dll",
            description="缺失 core dll 时，CLI 本地最小检查应 fail-fast。",
            mutate=lambda bin_dir: _remove_required(bin_dir / "tracer_core.dll"),
            expect_success=False,
            expected_exit=10,
            expected_tokens=["runtime check failed", "tracer_core.dll"],
            unexpected_tokens=[
                "configuration validation failed",
                "startup error",
            ],
        ),
        RuntimeGuardScenario(
            name="missing_config_toml",
            description="缺失 config.toml 时，core runtime-check 应返回缺文件错误。",
            mutate=lambda bin_dir: _remove_required(bin_dir / "config" / "config.toml"),
            expect_success=False,
            expected_exit=7,
            expected_tokens=["runtime check failed", "config.toml"],
            unexpected_tokens=[
                "configuration validation failed",
                "startup error",
            ],
        ),
        RuntimeGuardScenario(
            name="missing_reports_shared_dll",
            description="缺失 core 依赖 dll 时，core 动态加载应 fail-fast。",
            mutate=lambda bin_dir: _remove_required(bin_dir / "libreports_shared.dll"),
            expect_success=False,
            expected_exit=10,
            expected_tokens=["runtime check failed", "failed to load", "tracer_core.dll"],
            unexpected_tokens=[
                "configuration validation failed",
                "startup error",
            ],
        ),
    ]


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Run reusable runtime guard scenarios for tracer_windows_cli."
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
        help=("Workspace for guard cases. Default: test/output/tracer_windows_cli/runtime_guard"),
    )
    parser.add_argument(
        "--keep-workspace",
        action="store_true",
        help="Keep existing workspace content instead of cleaning before run.",
    )
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(sys.argv[1:] if argv is None else argv)
    repo_root = Path(__file__).resolve().parent.parent

    try:
        source_bin = _resolve_source_bin_dir(repo_root, args)
        _ensure_source_runtime_ready(source_bin)
    except Exception as error:
        print(f"[runtime-guard] setup failed: {error}", flush=True)
        return 2

    workspace_root = (
        Path(args.workspace).resolve()
        if args.workspace
        else (repo_root / "test" / "output" / "tracer_windows_cli" / "runtime_guard").resolve()
    )
    if workspace_root.exists() and not args.keep_workspace:
        shutil.rmtree(workspace_root)
    workspace_root.mkdir(parents=True, exist_ok=True)

    files_to_copy, folders_to_copy = _load_runtime_bundle_spec(repo_root)
    scenarios = _build_scenarios()
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


if __name__ == "__main__":
    raise SystemExit(main())

