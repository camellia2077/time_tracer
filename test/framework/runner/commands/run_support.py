import subprocess
from pathlib import Path

from tools.toolchain.core.generated_paths import resolve_build_layout


def resolve_app_root(repo_root: Path, app_name: str) -> Path:
    if app_name == "tracer_windows_rust_cli":
        return repo_root / "apps" / "tracer_cli" / "windows" / "rust_cli"
    return repo_root / "apps" / app_name


def resolve_runtime_platform_args(app_name: str) -> list[str]:
    if app_name == "tracer_windows_rust_cli":
        return ["--runtime-platform", "windows"]
    return []


def run_step(title: str, cmd: list[str], cwd: Path) -> int:
    print(f"\n=== {title} ===", flush=True)
    print("Command:", subprocess.list2cmdline(cmd), flush=True)
    completed = subprocess.run(cmd, cwd=str(cwd), check=False)
    if completed.returncode != 0:
        print(f"[FAILED] {title} (exit code: {completed.returncode})")
    else:
        print(f"[OK] {title}")
    return int(completed.returncode)


def default_build_dir(tidy: bool) -> str:
    return "build_tidy" if tidy else "build_fast"


def load_suite_default_build_dir(
    suite_root: Path,
    toml_loader,
) -> str | None:
    env_path = suite_root / "env.toml"
    if not env_path.exists():
        return None

    try:
        with env_path.open("rb") as file:
            data = toml_loader(file)
    except Exception as error:
        print(
            f"Warning: failed to read suite env.toml for default build dir: {env_path} ({error})",
            flush=True,
        )
        return None

    paths = data.get("paths", {})
    if not isinstance(paths, dict):
        return None

    default_build_dir = paths.get("default_build_dir")
    if not isinstance(default_build_dir, str):
        return None

    normalized = default_build_dir.strip()
    return normalized or None


def auto_detect_build_dir(repo_root: Path, app_name: str) -> str | None:
    candidates = ["build_fast", "build_agent", "build_tidy", "build"]
    for candidate in candidates:
        candidate_bin = resolve_build_layout(repo_root, app_name, candidate).bin_dir
        if candidate_bin.exists() and candidate_bin.is_dir():
            return candidate
    return None


def resolve_build_dir(
    repo_root: Path,
    app_name: str,
    requested_build_dir: str | None,
    requested_bin_dir: str | None,
    suite_default_build_dir: str | None,
    with_build: bool,
    skip_configure: bool,
    skip_build: bool,
    tidy: bool,
) -> str | None:
    if requested_bin_dir:
        return None

    if requested_build_dir:
        return requested_build_dir

    if suite_default_build_dir:
        print(
            f"Using suite default build dir from TOML: {suite_default_build_dir}",
            flush=True,
        )
        return suite_default_build_dir

    if with_build:
        return default_build_dir(tidy)

    if skip_configure and skip_build:
        detected = auto_detect_build_dir(repo_root=repo_root, app_name=app_name)
        if detected:
            print(f"Auto-detected build dir: {detected}", flush=True)
            return detected

    return auto_detect_build_dir(repo_root=repo_root, app_name=app_name)


def ensure_bin_dir_exists(
    repo_root: Path,
    app_name: str,
    build_dir: str | None,
    bin_dir: str | None,
) -> bool:
    if bin_dir:
        candidate = Path(bin_dir).resolve()
    elif build_dir:
        candidate = resolve_build_layout(repo_root, app_name, build_dir).bin_dir
    else:
        print("Error: no executable directory is resolved.")
        print("Hint: pass --build-dir or --bin-dir, or use --with-build.")
        return False

    if candidate.exists() and candidate.is_dir():
        return True

    print(f"Error: binary directory not found: {candidate}")
    print("Hint: run with --with-build, or set --build-dir/--bin-dir correctly.")
    return False


def run_optional_build_steps(
    repo_root: Path,
    app_name: str,
    scripts_run: Path,
    python_exe: str,
    effective_build_dir: str | None,
    tidy: bool,
    kill_build_procs: bool,
    skip_configure: bool,
    skip_build: bool,
) -> int:
    shared_build_flags: list[str] = []
    if tidy:
        shared_build_flags.append("--tidy")
    if kill_build_procs:
        shared_build_flags.append("--kill-build-procs")

    if not skip_configure:
        configure_cmd = [
            python_exe,
            str(scripts_run),
            "configure",
            "--app",
            app_name,
            *shared_build_flags,
        ]
        if effective_build_dir:
            configure_cmd.extend(["--build-dir", effective_build_dir])
        exit_code = run_step("Configure", configure_cmd, cwd=repo_root)
        if exit_code != 0:
            return exit_code

    if not skip_build:
        build_cmd = [
            python_exe,
            str(scripts_run),
            "build",
            "--app",
            app_name,
            *resolve_runtime_platform_args(app_name),
            *shared_build_flags,
        ]
        if effective_build_dir:
            build_cmd.extend(["--build-dir", effective_build_dir])
        exit_code = run_step("Build", build_cmd, cwd=repo_root)
        if exit_code != 0:
            return exit_code

    return 0
