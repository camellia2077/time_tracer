import re
from pathlib import Path
from unittest import TestCase


class TestShellTargetNamingPolicy(TestCase):
    _SHELL_TARGET_PREFIXES = (
        "tc_c_api_",
        "tt_android_runtime_",
        "tt_file_crypto_",
        "tt_exchange_runtime_",
    )
    _ALLOWED_LEGACY_NON_OWNER_TARGETS = {
        "tc_c_api_smoke_tests",
        "tt_android_runtime_shell_smoke_tests",
        "tt_file_crypto_runtime_bridge_tests",
    }
    _REQUIRED_TOKENS = (
        "pipeline",
        "query",
        "reporting",
        "exchange",
        "config",
        "persistence_runtime",
        "persistence_write",
        "shell_aggregate",
    )

    def test_new_shell_targets_must_carry_capability_or_shell_aggregate_token(self):
        repo_root = Path(__file__).resolve().parents[4]
        cmake_files = [
            repo_root / "apps" / "tracer_core_shell" / "cmake" / "CoreTargets.cmake",
            repo_root / "apps" / "tracer_core_shell" / "api" / "c_api" / "CMakeLists.txt",
        ]
        target_pattern = re.compile(r"add_executable\(([\w_]+)")

        offenders: list[str] = []
        for path in cmake_files:
            text = path.read_text(encoding="utf-8")
            for target in target_pattern.findall(text):
                if not target.startswith(self._SHELL_TARGET_PREFIXES):
                    continue
                if target in self._ALLOWED_LEGACY_NON_OWNER_TARGETS:
                    continue
                if not any(token in target for token in self._REQUIRED_TOKENS):
                    offenders.append(target)

        self.assertEqual(offenders, [])
