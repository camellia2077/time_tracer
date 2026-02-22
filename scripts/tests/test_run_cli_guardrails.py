import ast
import sys
from pathlib import Path
from unittest import TestCase

REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPTS_DIR = REPO_ROOT / "scripts"
RUN_SCRIPT = SCRIPTS_DIR / "run.py"

if str(SCRIPTS_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPTS_DIR))


class TestRunCliGuardrails(TestCase):
    def test_run_py_does_not_perform_build_filesystem_mutations(self):
        source = RUN_SCRIPT.read_text(encoding="utf-8")
        tree = ast.parse(source)

        forbidden_names = {
            "open",
            "remove",
            "rename",
            "replace",
            "unlink",
            "rmdir",
            "makedirs",
        }
        forbidden_attrs = {
            "mkdir",
            "write_text",
            "write_bytes",
            "unlink",
            "rmdir",
            "rename",
            "replace",
            "touch",
        }

        violations: list[str] = []
        for node in ast.walk(tree):
            if not isinstance(node, ast.Call):
                continue
            if isinstance(node.func, ast.Name):
                if node.func.id in forbidden_names:
                    violations.append(node.func.id)
                continue
            if isinstance(node.func, ast.Attribute):
                if node.func.attr in forbidden_attrs:
                    violations.append(node.func.attr)

        self.assertEqual(
            violations,
            [],
            f"run.py must stay as CLI dispatcher only, found fs mutation calls: {violations}",
        )
