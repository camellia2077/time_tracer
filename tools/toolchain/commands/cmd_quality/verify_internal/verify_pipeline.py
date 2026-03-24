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
    run_report_markdown_gates_fn,
    run_native_core_runtime_tests_fn,
) -> int:
    if test_cmd is None:
        print(
            f"--- verify: no mapped test suite for app `{app_name}`. "
            "Skipping suite step and continuing with applicable gates/runtime checks."
        )
    else:
        suite_ret = run_command_fn(
            test_cmd,
            cwd=repo_root,
            env=setup_env_fn(),
        )
        if suite_ret != 0:
            return suite_ret

    markdown_gate_ret = run_report_markdown_gates_fn(
        repo_root=repo_root,
        setup_env_fn=setup_env_fn,
        run_command_fn=run_command_fn,
        app_name=app_name,
        build_dir_name=build_dir_name,
        profile_name=profile_name,
    )
    if markdown_gate_ret != 0:
        return markdown_gate_ret

    native_ret = run_native_core_runtime_tests_fn(
        repo_root=repo_root,
        setup_env_fn=setup_env_fn,
        run_command_fn=run_command_fn,
        app_name=app_name,
        build_dir_name=build_dir_name,
        profile_name=profile_name,
    )
    if native_ret != 0:
        return native_ret
    return 0
