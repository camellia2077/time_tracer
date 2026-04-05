from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import TestCase

from tools.toolchain.commands.tidy.command_execute import execute_tidy_command


class _FakeTidyCommand:
    def __init__(self, root: Path, *, build_ret: int):
        self.root = root
        self.build_ret = build_ret
        self.split_called = False
        self.print_summary_called = False
        self.received_compile_units = None

    def _resolve_tidy_paths(self, _app_name: str, build_dir_name: str | None = None) -> dict[str, Path]:
        build_dir = self.root / (build_dir_name or "build_tidy")
        log_path = build_dir / "build.log"
        tasks_dir = build_dir / "tasks"
        ninja_log_path = build_dir / ".ninja_log"
        build_dir.mkdir(parents=True, exist_ok=True)
        tasks_dir.mkdir(parents=True, exist_ok=True)
        return {
            "build_dir": build_dir,
            "log_path": log_path,
            "tasks_dir": tasks_dir,
            "ninja_log_path": ninja_log_path,
        }

    def _ensure_configured(self, **_kwargs) -> tuple[int, bool, float]:
        return 0, False, 0.0

    def _resolve_build_options(self, extra_args, jobs, keep_going, *, job_mode: str = "full"):
        _ = job_mode
        return extra_args or [], False, jobs, bool(keep_going)

    def _build_tidy_command(
        self,
        _app_name: str,
        _build_dir: Path,
        filtered_args: list[str],
        _has_target_override: bool,
        _effective_jobs: int | None,
        _effective_keep_going: bool,
    ) -> list[str]:
        return ["cmake", "--build", *filtered_args]

    def _run_tidy_build(self, _cmd: list[str], log_path: Path, *, output_mode: str = "live") -> tuple[int, float]:
        _ = output_mode
        log_path.write_text("[1/1] warning: demo\n", encoding="utf-8")
        return self.build_ret, 1.25

    def _ensure_analysis_compile_db(self, build_dir: Path) -> Path:
        compile_db_dir = build_dir / "analysis"
        compile_db_dir.mkdir(parents=True, exist_ok=True)
        (compile_db_dir / "compile_commands.json").write_text("[]", encoding="utf-8")
        return compile_db_dir

    def _split_from_log(
        self,
        log_path: Path,
        tasks_dir: Path,
        *,
        parse_workers: int | None = None,
        task_view: str | None = None,
        workspace_name: str = "",
        source_scope: str | None = None,
        compile_units=None,
    ) -> tuple[dict, float]:
        _ = parse_workers, task_view, workspace_name, source_scope
        self.split_called = True
        self.received_compile_units = compile_units
        (tasks_dir / "tasks_summary.md").write_text(log_path.read_text(encoding="utf-8"), encoding="utf-8")
        return {"tasks": 1, "batches": 1, "batch_size": 10}, 0.2

    def _read_ninja_timing(self, _ninja_log_path: Path) -> dict | None:
        return None

    def _print_timing_summary(self, **_kwargs) -> None:
        self.print_summary_called = True


class TestTidyCommandExecute(TestCase):
    def test_returns_original_nonzero_ret_after_successful_log_split(self):
        with TemporaryDirectory() as temp_dir:
            command = _FakeTidyCommand(Path(temp_dir), build_ret=7)

            ret = execute_tidy_command(
                command=command,
                app_name="tracer_core_shell",
                build_dir_name="build_tidy_core_family",
                source_scope="core_family",
            )

        self.assertEqual(ret, 7)
        self.assertTrue(command.split_called)
        self.assertTrue(command.print_summary_called)
        self.assertEqual(command.received_compile_units, [])

    def test_returns_zero_after_successful_build_and_log_split(self):
        with TemporaryDirectory() as temp_dir:
            command = _FakeTidyCommand(Path(temp_dir), build_ret=0)

            ret = execute_tidy_command(
                command=command,
                app_name="tracer_core_shell",
                build_dir_name="build_tidy_core_family",
                source_scope="core_family",
            )

        self.assertEqual(ret, 0)
        self.assertTrue(command.split_called)
