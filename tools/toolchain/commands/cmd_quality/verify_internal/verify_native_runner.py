from __future__ import annotations

import os
from pathlib import Path

from ....core.generated_paths import resolve_build_layout
from ....services.suite_registry import resolve_result_output_name

_DEFAULT_NATIVE_TESTS = [
    "tc_c_api_smoke_tests",
    "tc_c_api_stability_tests",
    "tt_aggregate_runtime_tests",
    "tc_app_aggregate_runtime_smoke_tests",
    "tt_android_runtime_shell_smoke_tests",
]

_PROFILE_NATIVE_TESTS = {
    "cap_pipeline": [
        "tt_pipeline_api_tests",
        "tc_app_pipeline_mod_smoke_tests",
        "tc_app_workflow_mod_smoke_tests",
        "tt_android_runtime_pipeline_regression_tests",
    ],
    "cap_query": [
        "tt_query_api_tests",
        "tc_app_query_mod_smoke_tests",
        "tc_query_infra_smoke_tests",
        "tt_android_runtime_query_tests",
    ],
    "cap_reporting": [
        "tt_reporting_api_tests",
        "tc_reporting_infra_smoke_tests",
        "tt_fmt_parity_tests",
        "tt_android_runtime_reporting_tests",
    ],
    "cap_exchange": [
        "tt_exchange_api_tests",
        "tc_exchange_infra_smoke_tests",
        "tt_file_crypto_tests",
    ],
    "cap_config": [
        "tc_config_infra_smoke_tests",
        "tt_android_runtime_config_tests",
    ],
    "cap_persistence_runtime": [
        "tc_persistence_runtime_infra_smoke_tests",
    ],
    "cap_persistence_write": [
        "tc_persistence_write_infra_smoke_tests",
    ],
}


def run_native_core_runtime_tests(
    repo_root: Path,
    setup_env_fn,
    run_command_fn,
    app_name: str,
    build_dir_name: str,
    profile_name: str | None = None,
) -> int:
    if resolve_result_output_name(app_name) != "artifact_windows_cli":
        return 0

    bin_dir = resolve_build_layout(
        repo_root,
        "tracer_core_shell",
        build_dir_name,
    ).bin_dir
    suffix = ".exe" if os.name == "nt" else ""
    normalized_profile = (profile_name or "").strip().lower()
    tests = list(_PROFILE_NATIVE_TESTS.get(normalized_profile, _DEFAULT_NATIVE_TESTS))

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
