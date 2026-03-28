import tempfile
from pathlib import Path
from unittest import TestCase
from unittest.mock import patch

from tools.toolchain.core.path_display import to_repo_relative


class TestValidatePathDisplay(TestCase):
    def test_to_repo_relative_uses_equivalent_parent_when_direct_relative_to_fails(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            plan_path = repo_root / "temp" / "validate.toml"
            plan_path.parent.mkdir(parents=True, exist_ok=True)
            plan_path.write_text("demo = true\n", encoding="utf-8")

            alias_root = repo_root.parent / f"{repo_root.name}_alias"

            def fake_same_location(left: Path, right: Path) -> bool:
                resolved_left = left.resolve(strict=False)
                resolved_right = right.resolve(strict=False)
                return {
                    str(resolved_left),
                    str(resolved_right),
                } == {
                    str(repo_root.resolve(strict=False)),
                    str(alias_root.resolve(strict=False)),
                }

            with patch(
                "tools.toolchain.core.path_display._same_location",
                side_effect=fake_same_location,
            ):
                self.assertEqual(
                    to_repo_relative(alias_root, plan_path),
                    "temp/validate.toml",
                )
