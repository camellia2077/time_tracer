from pathlib import Path
from unittest import TestCase

REPO_ROOT = Path(__file__).resolve().parents[3]
RUST_CLI_SRC = REPO_ROOT / "apps" / "tracer_cli" / "windows" / "rust_cli" / "src"


class TestRustCliIndependenceGuardrails(TestCase):
    def test_rust_cli_runtime_must_not_depend_on_cpp_cli(self):
        forbidden_tokens = [
            "time_tracer_cli.exe",
            "should_passthrough_cpp",
            "passthrough_to_cpp",
            "resolve_cpp_cli_executable",
        ]
        violations: list[str] = []

        for file_path in RUST_CLI_SRC.rglob("*.rs"):
            content = file_path.read_text(encoding="utf-8")
            for token in forbidden_tokens:
                if token in content:
                    relative = file_path.relative_to(REPO_ROOT).as_posix()
                    violations.append(f"{relative}: {token}")

        self.assertEqual(
            violations,
            [],
            "Rust CLI runtime must stay independent from legacy C++ CLI. "
            f"Found forbidden tokens: {violations}",
        )
