from __future__ import annotations

VERIFY_STACK_TESTS: tuple[str, ...] = (
    "tools.tests.platform.core.test_capability_boundary_scan",
    "tools.tests.platform.core.test_context_config_resolution",
    "tools.tests.platform.core.test_core_boundary_policy",
    "tools.tests.platform.core.test_platform_config_sync",
    "tools.tests.platform.core.test_test_runner_build_support",
    "tools.tests.platform.tidy.test_tidy_task_auto_fix_apply",
    "tools.tests.platform.tidy.test_tidy_task_auto_fix_orchestrator",
    "tools.tests.platform.tidy.test_tidy_task_auto_fix_plan",
    "tools.tests.platform.tidy.test_tidy_task_auto_fix_rule_contract",
    "tools.tests.platform.tidy.test_tidy_task_builder_split",
    "tools.tests.platform.tidy.test_tidy_task_collectors",
    "tools.tests.platform.tidy.test_tidy_task_context",
    "tools.tests.platform.tidy.test_tidy_task_log_contract",
    "tools.tests.platform.tidy.test_tidy_task_render",
    "tools.tests.platform.tidy.test_tidy_step",
    "tools.tests.validate.test_validate_plan",
    "tools.tests.validate.test_validate_command",
    "tools.tests.validate.test_validate_cli_handler",
    "tools.tests.verify.test_capability_smoke_profiles",
    "tools.tests.verify.test_verify_markdown_gate_runner",
    "tools.tests.verify.test_verify_profile_inference",
    "tools.tests.verify.test_verify_native_runner",
    "tools.tests.verify.test_verify_pipeline",
    "tools.tests.verify.test_verify_result_writer",
    "tools.tests.verify.test_verify_run_tests",
    "tools.tests.verify.test_verify_execute_flow",
    "tools.tests.verify.test_verify_cli_handler",
    "tools.tests.run_cli.test_run_cli_dispatch_core_analyze",
    "tools.tests.run_cli.test_run_cli_dispatch_core_build_verify",
    "tools.tests.run_cli.test_run_cli_dispatch_core_misc",
    "tools.tests.run_cli.test_run_cli_dispatch_core_validate_format",
    "tools.tests.run_cli.test_run_cli_dispatch_tidy_queue",
    "tools.tests.run_cli.test_run_cli_dispatch_tidy_step",
    "tools.tests.run_cli.test_run_cli_dispatch_tidy_task",
)

PLATFORM_TESTS: tuple[str, ...] = (
    "tools.tests.platform.core.test_capability_boundary_scan",
    "tools.tests.platform.core.test_context_config_resolution",
    "tools.tests.platform.core.test_core_boundary_policy",
    "tools.tests.platform.core.test_platform_config_sync",
    "tools.tests.platform.core.test_test_runner_build_support",
    "tools.tests.platform.tidy.test_tidy_task_auto_fix_apply",
    "tools.tests.platform.tidy.test_tidy_task_auto_fix_orchestrator",
    "tools.tests.platform.tidy.test_tidy_task_auto_fix_plan",
    "tools.tests.platform.tidy.test_tidy_task_auto_fix_rule_contract",
    "tools.tests.platform.tidy.test_tidy_task_builder_split",
    "tools.tests.platform.tidy.test_tidy_task_collectors",
    "tools.tests.platform.tidy.test_tidy_task_context",
    "tools.tests.platform.tidy.test_tidy_task_log_contract",
    "tools.tests.platform.tidy.test_tidy_task_render",
    "tools.tests.platform.tidy.test_tidy_step",
)

VALIDATE_TESTS: tuple[str, ...] = (
    "tools.tests.validate.test_validate_plan",
    "tools.tests.validate.test_validate_command",
    "tools.tests.validate.test_validate_cli_handler",
)

VERIFY_TESTS: tuple[str, ...] = (
    "tools.tests.verify.test_capability_smoke_profiles",
    "tools.tests.verify.test_verify_markdown_gate_runner",
    "tools.tests.verify.test_verify_profile_inference",
    "tools.tests.verify.test_verify_native_runner",
    "tools.tests.verify.test_verify_pipeline",
    "tools.tests.verify.test_verify_result_writer",
    "tools.tests.verify.test_verify_run_tests",
    "tools.tests.verify.test_verify_execute_flow",
    "tools.tests.verify.test_verify_cli_handler",
    "tools.tests.verify.test_self_test_command",
)

RUN_CLI_TESTS: tuple[str, ...] = (
    "tools.tests.run_cli.test_run_cli_dispatch_core_analyze",
    "tools.tests.run_cli.test_run_cli_dispatch_core_build_verify",
    "tools.tests.run_cli.test_run_cli_dispatch_core_misc",
    "tools.tests.run_cli.test_run_cli_dispatch_core_validate_format",
    "tools.tests.run_cli.test_run_cli_dispatch_tidy_queue",
    "tools.tests.run_cli.test_run_cli_dispatch_tidy_step",
    "tools.tests.run_cli.test_run_cli_dispatch_tidy_task",
)

TEST_GROUPS: dict[str, tuple[str, ...]] = {
    "verify-stack": VERIFY_STACK_TESTS,
    "platform": PLATFORM_TESTS,
    "validate": VALIDATE_TESTS,
    "verify": VERIFY_TESTS,
    "run-cli": RUN_CLI_TESTS,
}
