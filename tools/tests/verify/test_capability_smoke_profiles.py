import json
from unittest import TestCase

from tools.toolchain.ci.capability_smoke_profiles import build_payload


class TestCapabilitySmokeProfiles(TestCase):
    def test_build_payload_returns_focused_profiles_for_owner_paths(self):
        payload = build_payload(
            "pull_request",
            [
                "apps/tracer_core_shell/api/c_api/capabilities/query/tracer_core_c_api_query.cpp",
                "apps/tracer_core_shell/api/c_api/runtime/tracer_core_c_api_internal.cpp",
            ],
        )

        self.assertTrue(payload["run_heavy_verify"])
        self.assertTrue(payload["run_capability_smoke"])
        self.assertEqual(payload["mode"], "focused")
        self.assertEqual(payload["profiles"], ["cap_query", "shell_aggregate"])
        self.assertEqual(
            json.loads(payload["matrix"]),
            {"include": [{"profile": "cap_query"}, {"profile": "shell_aggregate"}]},
        )

    def test_build_payload_falls_back_to_fast_for_shared_paths(self):
        payload = build_payload(
            "push",
            [
                "tools/toolchain/commands/cmd_quality/verify.py",
            ],
        )

        self.assertTrue(payload["run_heavy_verify"])
        self.assertFalse(payload["run_capability_smoke"])
        self.assertEqual(payload["mode"], "fast_fallback")
        self.assertEqual(payload["profiles"], [])
        self.assertIn("shared build/test infra", payload["reason"])

    def test_build_payload_maps_shell_runtime_helper_to_shell_aggregate(self):
        payload = build_payload(
            "pull_request",
            [
                "apps/tracer_core_shell/tests/integration/tracer_core_c_api_stability_internal.hpp",
            ],
        )

        self.assertTrue(payload["run_heavy_verify"])
        self.assertTrue(payload["run_capability_smoke"])
        self.assertEqual(payload["mode"], "focused")
        self.assertEqual(payload["profiles"], ["shell_aggregate"])

    def test_build_payload_shared_shell_cmake_path_falls_back_to_fast(self):
        payload = build_payload(
            "push",
            [
                "apps/tracer_core_shell/cmake/CoreTargets.cmake",
            ],
        )

        self.assertTrue(payload["run_heavy_verify"])
        self.assertFalse(payload["run_capability_smoke"])
        self.assertEqual(payload["mode"], "fast_fallback")
        self.assertIn("shared build/test infra", payload["reason"])

    def test_build_payload_skips_when_only_non_code_paths_change(self):
        payload = build_payload(
            "pull_request",
            [
                "README.md",
                "docs/time_tracer/history/v0.9/0.9.1.md",
            ],
        )

        self.assertFalse(payload["run_heavy_verify"])
        self.assertFalse(payload["run_capability_smoke"])
        self.assertFalse(payload["code_changes"])
        self.assertEqual(payload["mode"], "skip")

    def test_build_payload_keeps_schedule_on_heavy_only(self):
        payload = build_payload("schedule", [])

        self.assertTrue(payload["run_heavy_verify"])
        self.assertFalse(payload["run_capability_smoke"])
        self.assertEqual(payload["mode"], "heavy_only")
