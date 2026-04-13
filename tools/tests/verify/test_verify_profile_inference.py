from pathlib import Path
from unittest import TestCase
from unittest.mock import patch

from tools.toolchain.commands.cmd_quality.verify_internal.verify_profile_inference import (
    classify_changed_paths,
    infer_verify_profiles,
    list_worktree_changed_paths,
)


class TestVerifyProfileInference(TestCase):
    def test_classify_single_capability_path(self):
        inference = classify_changed_paths(
            ["apps/tracer_core_shell/api/c_api/capabilities/query/tracer_core_c_api_query.cpp"]
        )

        self.assertFalse(inference.fallback_to_fast)
        self.assertEqual(inference.profiles, ("cap_query",))

    def test_classify_multiple_capabilities_and_shell_aggregate_orders_shell_last(self):
        inference = classify_changed_paths(
            [
                "apps/tracer_core_shell/api/c_api/runtime/tracer_core_c_api_internal.cpp",
                "apps/tracer_core_shell/api/c_api/capabilities/pipeline/tracer_core_c_api_pipeline.cpp",
                "apps/tracer_core_shell/api/c_api/capabilities/query/tracer_core_c_api_query.cpp",
            ]
        )

        self.assertFalse(inference.fallback_to_fast)
        self.assertEqual(
            inference.profiles,
            ("cap_pipeline", "cap_query", "shell_aggregate"),
        )

    def test_classify_shared_toolchain_path_falls_back_to_fast(self):
        inference = classify_changed_paths(
            ["tools/toolchain/commands/cmd_quality/verify.py"]
        )

        self.assertTrue(inference.fallback_to_fast)
        self.assertEqual(inference.profiles, ("fast",))
        self.assertIn("shared build/test infra", inference.reason)

    def test_classify_suite_asset_path_falls_back_to_fast(self):
        inference = classify_changed_paths(
            ["tools/suites/tracer_windows_rust_cli/tests/commands_query_data.toml"]
        )

        self.assertTrue(inference.fallback_to_fast)
        self.assertEqual(inference.profiles, ("fast",))
        self.assertIn("shared build/test infra", inference.reason)

    def test_classify_test_framework_path_falls_back_to_fast(self):
        inference = classify_changed_paths(
            ["tools/test_framework/runner/service.py"]
        )

        self.assertTrue(inference.fallback_to_fast)
        self.assertEqual(inference.profiles, ("fast",))
        self.assertIn("shared build/test infra", inference.reason)

    def test_classify_unknown_path_falls_back_to_fast_as_unmapped(self):
        inference = classify_changed_paths(
            ["apps/tracer_core_shell/tests/platform/infrastructure/tests/android_runtime/unknown_new_bucket.cpp"]
        )

        self.assertTrue(inference.fallback_to_fast)
        self.assertEqual(inference.profiles, ("fast",))
        self.assertIn("unmapped paths", inference.reason)

    def test_classify_reporting_golden_path_maps_to_reporting(self):
        inference = classify_changed_paths(
            ["test/golden/report_formatter_parity/v1/day.md"]
        )

        self.assertFalse(inference.fallback_to_fast)
        self.assertEqual(inference.profiles, ("cap_reporting",))

    def test_classify_root_level_shell_runtime_helper_maps_to_shell_aggregate(self):
        inference = classify_changed_paths(
            [
                "apps/tracer_core_shell/tests/integration/tracer_core_c_api_stability_internal.hpp",
                "apps/tracer_core_shell/host/native_bridge_progress.cpp",
            ]
        )

        self.assertFalse(inference.fallback_to_fast)
        self.assertEqual(inference.profiles, ("shell_aggregate",))

    def test_classify_coretargets_cmake_falls_back_to_fast_with_shared_reason(self):
        inference = classify_changed_paths(
            ["apps/tracer_core_shell/cmake/CoreTargets.cmake"]
        )

        self.assertTrue(inference.fallback_to_fast)
        self.assertEqual(inference.profiles, ("fast",))
        self.assertIn("shared build/test infra", inference.reason)

    def test_classify_windows_workflow_falls_back_to_fast_with_shared_reason(self):
        inference = classify_changed_paths(
            [".github/workflows/windows-build-matrix.yml"]
        )

        self.assertTrue(inference.fallback_to_fast)
        self.assertEqual(inference.profiles, ("fast",))
        self.assertIn("shared build/test infra", inference.reason)

    def test_classify_capability_and_shell_aggregate_keeps_shell_last(self):
        inference = classify_changed_paths(
            [
                "apps/tracer_core_shell/tests/integration/tracer_core_c_api_stability_internal.hpp",
                "apps/tracer_core_shell/api/c_api/capabilities/reporting/tracer_core_c_api_reporting.cpp",
                "apps/tracer_core_shell/api/c_api/capabilities/query/tracer_core_c_api_query.cpp",
            ]
        )

        self.assertFalse(inference.fallback_to_fast)
        self.assertEqual(
            inference.profiles,
            ("cap_query", "cap_reporting", "shell_aggregate"),
        )

    @patch(
        "tools.toolchain.commands.cmd_quality.verify_internal.verify_profile_inference._run_git"
    )
    def test_list_worktree_changed_paths_uses_rename_destination_and_preserves_prefixes(
        self, mock_run_git
    ):
        mock_run_git.return_value = [
            " M .github/workflows/windows-build-matrix.yml",
            "RM apps/tracer_core_shell/api/c_api/tracer_core_c_api_reporting.cpp -> apps/tracer_core_shell/api/c_api/capabilities/query/tracer_core_c_api_query.cpp",
        ]

        paths = list_worktree_changed_paths(Path("C:/code/time_tracer"))

        self.assertEqual(
            paths,
            [
                ".github/workflows/windows-build-matrix.yml",
                "apps/tracer_core_shell/api/c_api/capabilities/query/tracer_core_c_api_query.cpp",
            ],
        )

    @patch(
        "tools.toolchain.commands.cmd_quality.verify_internal.verify_profile_inference.list_branch_changed_paths"
    )
    @patch(
        "tools.toolchain.commands.cmd_quality.verify_internal.verify_profile_inference.list_worktree_changed_paths"
    )
    def test_infer_verify_profiles_uses_branch_diff_when_worktree_is_clean(
        self, mock_worktree_paths, mock_branch_paths
    ):
        mock_worktree_paths.return_value = []
        mock_branch_paths.return_value = [
            "apps/tracer_core_shell/api/c_api/runtime/tracer_core_c_api_internal.cpp",
            "apps/tracer_core_shell/api/c_api/capabilities/query/tracer_core_c_api_query.cpp",
        ]

        inference = infer_verify_profiles(Path("C:/code/time_tracer"))

        self.assertFalse(inference.fallback_to_fast)
        self.assertEqual(inference.profiles, ("cap_query", "shell_aggregate"))
