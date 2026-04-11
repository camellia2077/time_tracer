from ....core.context import Context
from ....core.executor import run_command
from ....services.suite_registry import (
    needs_suite_build,
    resolve_suite_name,
    resolve_suite_runner_name,
)
from ...cmd_build import BuildCommand


def ensure_build_configured(ctx: Context, app_name: str, kill_build_procs: bool) -> int:
    build_dir = ctx.get_build_dir(app_name, "build_fast")
    if (build_dir / "CMakeCache.txt").exists():
        return 0

    print("--- tidy-loop: build_fast is not configured. Running configure...")
    builder = BuildCommand(ctx)
    return builder.configure(
        app_name=app_name,
        tidy=False,
        extra_args=None,
        kill_build_procs=kill_build_procs,
    )


def verify_loop(ctx: Context, app_name: str, concise: bool, kill_build_procs: bool) -> int:
    ret = ensure_build_configured(ctx=ctx, app_name=app_name, kill_build_procs=kill_build_procs)
    if ret != 0:
        return ret

    builder = BuildCommand(ctx)
    ret = builder.build(
        app_name=app_name,
        tidy=False,
        extra_args=None,
        kill_build_procs=kill_build_procs,
    )
    if ret != 0:
        return ret

    suite_name = resolve_suite_name(app_name)
    suite_runner_name = resolve_suite_runner_name(app_name)
    if not suite_name or not suite_runner_name:
        print(f"--- tidy-loop: no mapped test suite for app `{app_name}`; skip suite verify.")
        return 0

    test_cmd = [
        "python",
        "tools/test.py",
        "suite",
        "--suite",
        suite_runner_name,
        "--agent",
        "--build-dir",
        "build_fast",
    ]
    if needs_suite_build(app_name):
        test_cmd.extend(["--with-build", "--skip-configure"])
    if concise:
        test_cmd.append("--concise")
    return run_command(
        test_cmd,
        cwd=ctx.repo_root,
        env=ctx.setup_env(),
    )
