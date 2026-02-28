# test/infrastructure/workspace.py
import os
import shutil
from pathlib import Path
from tempfile import TemporaryDirectory

from ..conf.definitions import Colors


def _color(text: str) -> str:
    return "" if os.environ.get("TT_TEST_NO_COLOR") == "1" else text


class Workspace:
    def __init__(self, use_temp: bool = True):
        self.use_temp = use_temp
        self._temp_obj = None

    def setup(self, target_dir_override: Path | None = None, should_clean: bool = False) -> Path:
        if self.use_temp:
            self._temp_obj = TemporaryDirectory(prefix="tt_test_")
            root = Path(self._temp_obj.name)
            print(
                f"  {_color(Colors.GREEN)}Created temporary test environment: "
                f"{root}{_color(Colors.RESET)}"
            )
            return root

        if not target_dir_override:
            raise ValueError("target_dir_override is required in non-temporary mode.")

        if target_dir_override.exists() and should_clean:
            print(
                f"  {_color(Colors.YELLOW)}Cleaning test environment: "
                f"{target_dir_override}{_color(Colors.RESET)}"
            )
            try:
                for item in target_dir_override.iterdir():
                    if item.is_dir():
                        shutil.rmtree(item)
                    else:
                        item.unlink()
            except Exception as error:
                print(
                    f"  {_color(Colors.RED)}Failed to clean some files: "
                    f"{error}{_color(Colors.RESET)}"
                )

        target_dir_override.mkdir(parents=True, exist_ok=True)
        print(
            f"  {_color(Colors.YELLOW)}Using fixed test environment: "
            f"{target_dir_override}{_color(Colors.RESET)}"
        )
        return target_dir_override

    def teardown(self):
        if self._temp_obj:
            self._temp_obj.cleanup()
