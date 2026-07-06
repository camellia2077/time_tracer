import tempfile
from pathlib import Path
from unittest import TestCase

from tools.toolchain.core.process_lock import hold_process_lock


class TestProcessLock(TestCase):
    def test_hold_process_lock_removes_lock_file_after_release(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            lock_path = Path(temp_dir) / "locks" / "android_gradle.lock"

            with hold_process_lock(lock_path=lock_path, label="test lock"):
                self.assertTrue(lock_path.exists())

            self.assertFalse(lock_path.exists())
