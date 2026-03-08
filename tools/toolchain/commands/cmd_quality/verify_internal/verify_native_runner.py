from __future__ import annotations

import os
from pathlib import Path

from ....core.generated_paths import resolve_build_layout
from ....services.suite_registry import resolve_result_output_name


def run_native_core_runtime_tests(
    repo_root: Path,
    setup_env_fn,
    run_command_fn,
    app_name: str,
    build_dir_name: str,
) -> int:
    if resolve_result_output_name(app_name) != "artifact_windows_cli":
        return 0

    bin_dir = resolve_build_layout(
        repo_root,
        "tracer_core_shell",
        build_dir_name,
    ).bin_dir
    suffix = ".exe" if os.name == "nt" else ""
    tests = [
        "tc_c_api_smoke_tests",
        "tc_c_api_stability_tests",
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
