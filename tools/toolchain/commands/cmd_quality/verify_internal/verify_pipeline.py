from __future__ import annotations


def run_artifact_pipeline(
    *,
    test_cmd: list[str] | None,
    app_name: str,
    build_dir_name: str,
    profile_name: str | None,
    repo_root,
    setup_env_fn,
    run_command_fn,
    run_insights_markdown_gates_fn,
    run_native_core_runtime_tests_fn,
    run_native: bool = True,
    run_host_blackbox: bool = True,
    run_quality_gates: bool = True,
    record_phase_fn=None,
) -> int:
    print("--- verify: artifact phase [native_foundation]")
    native_ret = 0
    if run_native:
        native_ret = run_native_core_runtime_tests_fn(
            repo_root=repo_root,
            setup_env_fn=setup_env_fn,
            run_command_fn=run_command_fn,
            app_name=app_name,
            build_dir_name=build_dir_name,
            profile_name=profile_name,
            record_phase_fn=record_phase_fn,
        )
    else:
        print("--- verify: native foundation skipped by selected scope")
        if record_phase_fn is not None:
            record_phase_fn(
                name="artifact.native_foundation",
                category="artifact",
                status="skipped",
                exit_code=0,
            )
    if native_ret != 0:
        if record_phase_fn is not None:
            record_phase_fn(
                name="artifact.native_foundation",
                category="artifact",
                status="failed",
                exit_code=native_ret,
            )
        return native_ret
    if run_native and record_phase_fn is not None:
        record_phase_fn(
            name="artifact.native_foundation",
            category="artifact",
            status="passed",
            exit_code=0,
        )

    print("--- verify: artifact phase [host_blackbox]")
    if not run_host_blackbox:
        print("--- verify: host blackbox skipped by selected scope")
        if record_phase_fn is not None:
            record_phase_fn(
                name="artifact.host_blackbox",
                category="artifact",
                status="skipped",
                exit_code=0,
            )
    elif test_cmd is None:
        print(
            f"--- verify: no mapped test suite for app `{app_name}`. "
            "Skipping suite step and continuing with applicable gates/runtime checks."
        )
        if record_phase_fn is not None:
            record_phase_fn(
                name="artifact.host_blackbox",
                category="artifact",
                status="skipped",
                exit_code=0,
            )
    else:
        suite_ret = run_command_fn(
            test_cmd,
            cwd=repo_root,
            env=setup_env_fn(),
        )
        if suite_ret != 0:
            if record_phase_fn is not None:
                record_phase_fn(
                    name="artifact.host_blackbox",
                    category="artifact",
                    status="failed",
                    exit_code=suite_ret,
                )
            return suite_ret
        if record_phase_fn is not None:
            record_phase_fn(
                name="artifact.host_blackbox",
                category="artifact",
                status="passed",
                exit_code=0,
            )

    print("--- verify: artifact phase [golden_or_quality_gates]")
    if not run_quality_gates:
        print("--- verify: quality gates skipped by selected scope")
        if record_phase_fn is not None:
            record_phase_fn(
                name="artifact.golden_or_quality_gates",
                category="artifact",
                status="skipped",
                exit_code=0,
            )
        return 0
    markdown_gate_ret = run_insights_markdown_gates_fn(
        repo_root=repo_root,
        setup_env_fn=setup_env_fn,
        run_command_fn=run_command_fn,
        app_name=app_name,
        build_dir_name=build_dir_name,
        profile_name=profile_name,
    )
    if markdown_gate_ret != 0:
        if record_phase_fn is not None:
            record_phase_fn(
                name="artifact.golden_or_quality_gates",
                category="artifact",
                status="failed",
                exit_code=markdown_gate_ret,
            )
        return markdown_gate_ret
    if record_phase_fn is not None:
        record_phase_fn(
            name="artifact.golden_or_quality_gates",
            category="artifact",
            status="passed",
            exit_code=0,
        )
    return 0
