from __future__ import annotations

import os
from pathlib import Path

from ....core.generated_paths import resolve_build_layout
from ....services.suite_registry import resolve_result_output_name

_DEFAULT_NATIVE_PHASES = (
    (
        "c_abi_contract",
        (
            "tc_c_api_smoke_tests",
            "tc_c_api_shell_aggregate_tests",
        ),
    ),
    (
        "transport_contract",
        (
            "ttr_tests",
            "ttr_rt_codec_tests",
        ),
    ),
    (
        "runtime_smoke_and_bridge",
        (
            "tt_aggregate_runtime_tests",
            "tc_app_aggregate_runtime_smoke_tests",
            "tt_android_runtime_shell_smoke_tests",
            "tt_file_crypto_runtime_bridge_tests",
        ),
    ),
)

_PROFILE_NATIVE_PHASES = {
    "cap_pipeline": (
        ("core_semantics", ("tt_pipeline_api_tests",)),
        ("c_abi_contract", ("tc_c_api_pipeline_tests",)),
        (
            "runtime_smoke_and_bridge",
            (
                "tc_app_pipeline_mod_smoke_tests",
                "tc_app_workflow_mod_smoke_tests",
                "tt_android_runtime_pipeline_regression_tests",
            ),
        ),
    ),
    "cap_query": (
        ("core_semantics", ("tt_query_api_tests",)),
        ("c_abi_contract", ("tc_c_api_query_tests",)),
        (
            "runtime_smoke_and_bridge",
            (
                "tc_app_query_mod_smoke_tests",
                "tc_query_infra_smoke_tests",
                "tt_android_runtime_query_tests",
            ),
        ),
    ),
    "cap_reporting": (
        ("core_semantics", ("tt_reporting_api_tests",)),
        ("c_abi_contract", ("tc_c_api_reporting_tests",)),
        (
            "runtime_smoke_and_bridge",
            (
                "tc_reporting_infra_smoke_tests",
                "tt_android_runtime_reporting_tests",
            ),
        ),
        ("golden_or_parity", ("tt_fmt_parity_tests",)),
    ),
    "cap_exchange": (
        ("core_semantics", ("tt_exchange_api_tests",)),
        (
            "runtime_smoke_and_bridge",
            (
                "tc_exchange_infra_smoke_tests",
                "tt_exchange_runtime_tests",
            ),
        ),
    ),
    "cap_config": (
        (
            "runtime_smoke_and_bridge",
            (
                "tc_config_infra_smoke_tests",
                "tt_android_runtime_config_tests",
            ),
        ),
    ),
    "cap_persistence_runtime": (
        ("runtime_smoke_and_bridge", ("tc_persistence_runtime_infra_smoke_tests",)),
    ),
    "cap_persistence_write": (
        ("runtime_smoke_and_bridge", ("tc_persistence_write_infra_smoke_tests",)),
    ),
    "shell_aggregate": _DEFAULT_NATIVE_PHASES,
}


def run_native_core_runtime_tests(
    repo_root: Path,
    setup_env_fn,
    run_command_fn,
    app_name: str,
    build_dir_name: str,
    profile_name: str | None = None,
    record_phase_fn=None,
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
    phases = _PROFILE_NATIVE_PHASES.get(normalized_profile, _DEFAULT_NATIVE_PHASES)

    for phase_name, tests in phases:
        print(f"--- verify: native phase [{phase_name}]")
        ran_any = False
        for test_name in tests:
            executable = bin_dir / f"{test_name}{suffix}"
            if not executable.exists():
                print(
                    f"--- verify: skip native core runtime test `{test_name}` "
                    f"(missing executable: {executable})"
                )
                continue
            ran_any = True
            print(f"--- verify: running native core runtime test `{test_name}`")
            ret = run_command_fn(
                [str(executable)],
                cwd=bin_dir,
                env=setup_env_fn(),
            )
            if ret != 0:
                if record_phase_fn is not None:
                    record_phase_fn(
                        name=f"native.{phase_name}",
                        category="native",
                        status="failed",
                        exit_code=ret,
                    )
                return ret
        if record_phase_fn is not None:
            record_phase_fn(
                name=f"native.{phase_name}",
                category="native",
                status="passed" if ran_any else "skipped",
                exit_code=0,
            )
    return 0
