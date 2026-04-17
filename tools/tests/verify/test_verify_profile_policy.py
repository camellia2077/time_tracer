from unittest import TestCase

from tools.toolchain.commands.cmd_quality.verify_internal.verify_profile_policy import (
    resolve_suite_config_override,
)


class TestVerifyProfilePolicy(TestCase):
    def test_tracer_android_release_verify_uses_release_config(self):
        self.assertEqual(
            resolve_suite_config_override("tracer_android", "android_release_verify"),
            "config_android_release_verify.toml",
        )

    def test_tracer_android_ci_keeps_default_ci_config(self):
        self.assertEqual(
            resolve_suite_config_override("tracer_android", "android_ci"),
            "config_android_ci.toml",
        )
