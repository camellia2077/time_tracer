from __future__ import annotations

VERIFY_STACK_TESTS: tuple[str, ...] = (
    "tools.tests.platform.test_build_toolchain_flags",
    "tools.tests.platform.test_capability_boundary_scan",
    "tools.tests.platform.test_context_config_resolution",
    "tools.tests.platform.test_core_boundary_policy",
    "tools.tests.platform.test_platform_config_sync",
    "tools.tests.platform.test_tidy_task_automation",
    "tools.tests.platform.test_tidy_step",
    "tools.tests.validate.test_validate_plan",
    "tools.tests.validate.test_validate_command",
    "tools.tests.validate.test_validate_cli_handler",
    "tools.tests.verify.test_capability_smoke_profiles",
    "tools.tests.verify.test_verify_markdown_gate_runner",
    "tools.tests.verify.test_verify_profile_inference",
    "tools.tests.verify.test_verify_native_runner",
    "tools.tests.verify.test_verify_run_tests",
    "tools.tests.verify.test_verify_execute_flow",
    "tools.tests.verify.test_verify_cli_handler",
    "tools.tests.run_cli.test_run_cli_dispatch",
)

PLATFORM_TESTS: tuple[str, ...] = (
    "tools.tests.platform.test_build_toolchain_flags",
    "tools.tests.platform.test_capability_boundary_scan",
    "tools.tests.platform.test_context_config_resolution",
    "tools.tests.platform.test_core_boundary_policy",
    "tools.tests.platform.test_platform_config_sync",
    "tools.tests.platform.test_tidy_task_automation",
    "tools.tests.platform.test_tidy_step",
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
    "tools.tests.verify.test_verify_run_tests",
    "tools.tests.verify.test_verify_execute_flow",
    "tools.tests.verify.test_verify_cli_handler",
    "tools.tests.verify.test_self_test_command",
)

RUN_CLI_TESTS: tuple[str, ...] = (
    "tools.tests.run_cli.test_run_cli_dispatch",
)

TEST_GROUPS: dict[str, tuple[str, ...]] = {
    "verify-stack": VERIFY_STACK_TESTS,
    "platform": PLATFORM_TESTS,
    "validate": VALIDATE_TESTS,
    "verify": VERIFY_TESTS,
    "run-cli": RUN_CLI_TESTS,
}
