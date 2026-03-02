from __future__ import annotations

import time

from .verify_scope_runner import run_scope_checks


def run_scope_pipeline(
    *,
    verify_scope: str,
    run_task_scope_checks,
    run_unit_scope_checks,
    run_artifact_scope_checks,
    suite_name: str | None,
    app_name: str,
    resolved_build_dir_name: str,
    started_at: float,
    verify_command_text: str,
    repo_root,
    write_build_only_result_json_fn,
    print_failure_report_fn,
    print_result_paths_fn,
) -> int:
    suite_ret = run_scope_checks(
        verify_scope=verify_scope,
        run_task_scope_checks=run_task_scope_checks,
        run_unit_scope_checks=run_unit_scope_checks,
        run_artifact_scope_checks=run_artifact_scope_checks,
    )

    if not suite_name:
        write_build_only_result_json_fn(
            app_name=app_name,
            build_dir_name=resolved_build_dir_name,
            success=(suite_ret == 0),
            exit_code=suite_ret,
            duration_seconds=time.monotonic() - started_at,
            error_message=("Build-only verification failed." if suite_ret != 0 else ""),
        )

    if suite_ret != 0:
        print_failure_report_fn(
            command=verify_command_text,
            exit_code=suite_ret,
            next_action=f"Fix errors and rerun: {verify_command_text}",
            app_name=app_name,
            repo_root=repo_root,
            fallback_key_error_hint="Test gate failed. See command output above.",
        )

    print_result_paths_fn(app_name=app_name, repo_root=repo_root)
    return suite_ret


def run_artifact_pipeline(
    *,
    test_cmd: list[str] | None,
    app_name: str,
    build_dir_name: str,
    repo_root,
    setup_env_fn,
    run_command_fn,
    run_report_markdown_gates_fn,
    run_native_core_runtime_tests_fn,
) -> int:
    if test_cmd is None:
        print(
            f"--- verify: no mapped test suite for app `{app_name}`. "
            "Build-only verification completed."
        )
        return 0

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
    )
    if markdown_gate_ret != 0:
        return markdown_gate_ret

    native_ret = run_native_core_runtime_tests_fn(
        repo_root=repo_root,
        setup_env_fn=setup_env_fn,
        run_command_fn=run_command_fn,
        app_name=app_name,
        build_dir_name=build_dir_name,
    )
    if native_ret != 0:
        return native_ret
    return 0
