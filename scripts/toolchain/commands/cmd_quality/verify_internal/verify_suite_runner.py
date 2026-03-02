from __future__ import annotations

from collections.abc import Callable


def build_suite_test_command(
    *,
    app_name: str,
    build_dir_name: str,
    profile_name: str | None,
    concise: bool,
    skip_suite_build: bool,
    resolve_suite_name_fn: Callable[[str], str | None],
    resolve_suite_runner_name_fn: Callable[[str], str | None],
    resolve_suite_bin_dir_fn: Callable[[str], str | None],
    needs_suite_build_fn: Callable[[str], bool],
    resolve_suite_config_override_fn: Callable[[str, str | None], str | None],
) -> list[str] | None:
    suite_name = resolve_suite_name_fn(app_name)
    suite_runner_name = resolve_suite_runner_name_fn(app_name)
    if not suite_name or not suite_runner_name:
        return None

    test_cmd = [
        "python",
        "test/run.py",
        "suite",
        "--suite",
        suite_runner_name,
        "--agent",
    ]
    suite_bin_dir = resolve_suite_bin_dir_fn(app_name)
    if suite_bin_dir:
        test_cmd.extend(["--bin-dir", suite_bin_dir])
    else:
        test_cmd.extend(["--build-dir", build_dir_name])

    suite_config_override = resolve_suite_config_override_fn(suite_name, profile_name)
    if suite_config_override:
        test_cmd.extend(["--config", suite_config_override])

    if needs_suite_build_fn(app_name) and not skip_suite_build:
        test_cmd.extend(["--with-build", "--skip-configure"])
    if concise:
        test_cmd.append("--concise")

    return test_cmd
