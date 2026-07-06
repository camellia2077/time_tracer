import tempfile
from pathlib import Path
from unittest import TestCase
from unittest.mock import patch

from tools.toolchain.core.executor import run_command


class _FakeStdout:
    def __iter__(self):
        raise KeyboardInterrupt()


class _FakeProcess:
    def __init__(self) -> None:
        self.stdout = _FakeStdout()
        self.terminated = False
        self.killed = False

    def terminate(self) -> None:
        self.terminated = True

    def kill(self) -> None:
        self.killed = True

    def wait(self, timeout=None) -> int:
        _ = timeout
        return 130


class _FakeWindowsProcess(_FakeProcess):
    pid = 12345


class TestExecutor(TestCase):
    def test_run_command_returns_child_code_after_keyboard_interrupt(self) -> None:
        fake_process = _FakeProcess()

        with tempfile.TemporaryDirectory() as temp_dir:
            log_path = Path(temp_dir) / "command.log"
            with patch("tools.toolchain.core.executor.subprocess.Popen", return_value=fake_process):
                result = run_command(["python", "-c", "print('x')"], log_file=log_path)

            self.assertEqual(result, 130)
            self.assertTrue(fake_process.terminated)
            self.assertFalse(fake_process.killed)
            log_text = log_path.read_text(encoding="utf-8")
            self.assertIn("Command interrupted", log_text)

    def test_run_command_terminates_windows_process_tree_after_keyboard_interrupt(self) -> None:
        fake_process = _FakeWindowsProcess()

        with (
            patch("tools.toolchain.core.executor.sys.platform", "win32"),
            patch("tools.toolchain.core.executor.subprocess.Popen", return_value=fake_process),
            patch("tools.toolchain.core.executor.subprocess.run") as run_mock,
        ):
            result = run_command(["gradlew.bat", ":app:lintDebug"], output_mode="quiet")

        self.assertEqual(result, 130)
        self.assertFalse(fake_process.terminated)
        run_mock.assert_called_once()
        self.assertEqual(
            run_mock.call_args.args[0],
            ["taskkill", "/T", "/PID", "12345"],
        )
