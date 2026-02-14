import argparse
import subprocess
import sys
from pathlib import Path
try:
    import tomllib
except ImportError:  # pragma: no cover
    import tomli as tomllib  # type: ignore

test_root = Path(__file__).resolve().parent
framework_root = test_root / "framework"
sys.path.insert(0, str(framework_root))

from suite_runner import run_suite


SUITE_META = {
    "time_tracer": {
        "suite_name": "time_tracer",
        "description": "Executable business-logic test runner.",
        "format_app": "time_tracer",
    },
    "log_generator": {
        "suite_name": "log_generator",
        "description": "Executable business-logic test runner for log_generator.",
        "format_app": None,
    },
}

ALIASES = {
    "tt": "time_tracer",
    "lg": "log_generator",
}


def _run_step(title: str, cmd: list[str], cwd: Path) -> int:
    print(f"\n=== {title} ===", flush=True)
    print("Command:", subprocess.list2cmdline(cmd), flush=True)
    completed = subprocess.run(cmd, cwd=str(cwd), check=False)
    if completed.returncode != 0:
        print(f"[FAILED] {title} (exit code: {completed.returncode})")
    else:
        print(f"[OK] {title}")
    return int(completed.returncode)


def _default_build_dir(tidy: bool) -> str:
    return "build_tidy" if tidy else "build_fast"


def _load_suite_default_build_dir(suite_root: Path) -> str | None:
    env_path = suite_root / "env.toml"
    if not env_path.exists():
        return None

    try:
        with env_path.open("rb") as file:
            data = tomllib.load(file)
    except Exception as error:
        print(
            f"Warning: failed to read suite env.toml for default build dir: "
            f"{env_path} ({error})",
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


def _auto_detect_build_dir(repo_root: Path, app_name: str) -> str | None:
    app_root = repo_root / "apps" / app_name
    candidates = ["build_fast", "build_agent", "build_tidy", "build"]
    for candidate in candidates:
        candidate_bin = app_root / candidate / "bin"
        if candidate_bin.exists() and candidate_bin.is_dir():
            return candidate
    return None


def _resolve_build_dir(
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
        print(f"Using suite default build dir from TOML: {suite_default_build_dir}", flush=True)
        return suite_default_build_dir

    if with_build:
        return _default_build_dir(tidy)

    if skip_configure and skip_build:
        detected = _auto_detect_build_dir(repo_root=repo_root, app_name=app_name)
        if detected:
            print(f"Auto-detected build dir: {detected}", flush=True)
            return detected

    return _auto_detect_build_dir(repo_root=repo_root, app_name=app_name)


def _ensure_bin_dir_exists(
    repo_root: Path,
    app_name: str,
    build_dir: str | None,
    bin_dir: str | None,
) -> bool:
    if bin_dir:
        candidate = Path(bin_dir).resolve()
    elif build_dir:
        candidate = (repo_root / "apps" / app_name / build_dir / "bin").resolve()
    else:
        print("Error: no executable directory is resolved.")
        print("Hint: pass --build-dir or --bin-dir, or use --with-build.")
        return False

    if candidate.exists() and candidate.is_dir():
        return True

    print(f"Error: binary directory not found: {candidate}")
    print("Hint: run with --with-build, or set --build-dir/--bin-dir correctly.")
    return False


def parse_args(argv):
    parser = argparse.ArgumentParser(
        description="Unified test entry for all executable suites.")
    parser.add_argument(
        "--suite",
        default="time_tracer",
        choices=["time_tracer", "log_generator", "tt", "lg"],
        help="Suite to run. Default: time_tracer.",
    )
    parser.add_argument(
        "--with-build",
        action="store_true",
        help="Run configure + build before tests.",
    )
    parser.add_argument(
        "--skip-configure",
        action="store_true",
        help="Skip configure stage when --with-build is enabled.",
    )
    parser.add_argument(
        "--skip-build",
        action="store_true",
        help="Skip build stage when --with-build is enabled.",
    )
    parser.add_argument(
        "--tidy",
        action="store_true",
        help="Build with clang-tidy enabled (default build dir becomes build_tidy).",
    )
    parser.add_argument(
        "--kill-build-procs",
        action="store_true",
        help="Pass kill-build-procs to configure/build stages.",
    )
    parser.add_argument(
        "--build-dir",
        default=None,
        help=(
            "Build folder under apps/<app>, e.g. build/build_fast/build_tidy. "
            "If omitted, uses suite TOML [paths].default_build_dir when set."
        ),
    )
    parser.add_argument(
        "--bin-dir",
        default=None,
        help="Direct executable directory override (highest priority for tests).",
    )
    return parser.parse_known_args(argv)


def main(argv=None):
    args, forwarded = parse_args(sys.argv[1:] if argv is None else argv)
    suite_key = ALIASES.get(args.suite, args.suite)
    app_name = suite_key
    meta = SUITE_META[suite_key]
    suite_root = test_root / "suites" / suite_key
    repo_root = test_root.parent

    if not suite_root.exists():
        print(f"Suite folder not found: {suite_root}")
        return 1

    suite_default_build_dir = _load_suite_default_build_dir(suite_root)
    effective_build_dir = _resolve_build_dir(
        repo_root=repo_root,
        app_name=app_name,
        requested_build_dir=args.build_dir,
        requested_bin_dir=args.bin_dir,
        suite_default_build_dir=suite_default_build_dir,
        with_build=args.with_build,
        skip_configure=args.skip_configure,
        skip_build=args.skip_build,
        tidy=args.tidy,
    )

    if args.with_build:
        scripts_run = repo_root / "scripts" / "run.py"
        if not scripts_run.exists():
            print(f"Error: scripts runner not found: {scripts_run}")
            return 1

        python_exe = sys.executable
        shared_build_flags: list[str] = []
        if args.tidy:
            shared_build_flags.append("--tidy")
        if args.kill_build_procs:
            shared_build_flags.append("--kill-build-procs")

        if not args.skip_configure:
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
            exit_code = _run_step("Configure", configure_cmd, cwd=repo_root)
            if exit_code != 0:
                return exit_code

        if not args.skip_build:
            build_cmd = [
                python_exe,
                str(scripts_run),
                "build",
                "--app",
                app_name,
                *shared_build_flags,
            ]
            if effective_build_dir:
                build_cmd.extend(["--build-dir", effective_build_dir])
            exit_code = _run_step("Build", build_cmd, cwd=repo_root)
            if exit_code != 0:
                return exit_code

    if not _ensure_bin_dir_exists(
        repo_root=repo_root,
        app_name=app_name,
        build_dir=effective_build_dir,
        bin_dir=args.bin_dir,
    ):
        return 1

    forwarded_args = list(forwarded)
    if args.bin_dir:
        forwarded_args.extend(["--bin-dir", args.bin_dir])
    elif effective_build_dir:
        forwarded_args.extend(["--build-dir", effective_build_dir])

    return run_suite(
        argv=forwarded_args,
        suite_root=suite_root,
        suite_name=meta["suite_name"],
        description=meta["description"],
        format_app=meta["format_app"],
        test_root=test_root,
    )


if __name__ == "__main__":
    sys.exit(main())
