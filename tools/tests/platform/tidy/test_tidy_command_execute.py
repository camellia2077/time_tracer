from pathlib import Path
from tempfile import TemporaryDirectory
from unittest.mock import patch
from unittest import TestCase

from tools.toolchain.commands.tidy.command_execute import execute_tidy_command


class _FakeTidyServices:
    def __init__(self, root: Path, *, build_ret: int):
        self.root = root
        self.build_ret = build_ret
        self.print_summary_called = False

    def resolve_tidy_paths(self, _ctx, _app_name: str, build_dir_name: str | None = None) -> dict[str, Path]:
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

    def ensure_configured(self, *_args, **_kwargs) -> tuple[int, bool, float]:
        return 0, False, 0.0

    def resolve_build_options(self, _ctx, extra_args, jobs, keep_going, *, job_mode: str = "full"):
        _ = job_mode
        return extra_args or [], False, jobs, bool(keep_going)

    def build_tidy_command(
        self,
        _app_name: str,
        _build_dir: Path,
        filtered_args: list[str],
        _has_target_override: bool,
        _effective_jobs: int | None,
        _effective_keep_going: bool,
    ) -> list[str]:
        return ["cmake", "--build", *filtered_args]

    def run_tidy_build(self, _ctx, _cmd: list[str], log_path: Path, *, output_mode: str = "live") -> tuple[int, float]:
        _ = output_mode
        log_path.write_text("[1/1] warning: demo\n", encoding="utf-8")
        return self.build_ret, 1.25

    def ensure_analysis_compile_db(self, build_dir: Path) -> Path:
        compile_db_dir = build_dir / "analysis"
        compile_db_dir.mkdir(parents=True, exist_ok=True)
        (compile_db_dir / "compile_commands.json").write_text("[]", encoding="utf-8")
        return compile_db_dir

    def read_ninja_timing(self, _ninja_log_path: Path) -> dict | None:
        return None

    def print_timing_summary(self, **_kwargs) -> None:
        self.print_summary_called = True


class TestTidyCommandExecute(TestCase):
    def test_requires_structured_results(self):
        with TemporaryDirectory() as temp_dir:
            services = _FakeTidyServices(Path(temp_dir), build_ret=7)

            with patch(
                "tools.toolchain.commands.tidy.command_execute.tidy_invoker.resolve_tidy_paths",
                services.resolve_tidy_paths,
            ), patch(
                "tools.toolchain.commands.tidy.command_execute.tidy_invoker.ensure_configured",
                services.ensure_configured,
            ), patch(
                "tools.toolchain.commands.tidy.command_execute.tidy_invoker.resolve_build_options",
                services.resolve_build_options,
            ), patch(
                "tools.toolchain.commands.tidy.command_execute.tidy_invoker.build_tidy_command",
                services.build_tidy_command,
            ), patch(
                "tools.toolchain.commands.tidy.command_execute.tidy_invoker.run_tidy_build",
                services.run_tidy_build,
            ), patch(
                "tools.toolchain.commands.tidy.command_execute.analysis_compile_db.ensure_analysis_compile_db",
                services.ensure_analysis_compile_db,
            ), patch(
                "tools.toolchain.commands.tidy.command_execute.tidy_timing.read_ninja_timing",
                services.read_ninja_timing,
            ), patch(
                "tools.toolchain.commands.tidy.command_execute.tidy_timing.print_timing_summary",
                services.print_timing_summary,
            ):
                ret = execute_tidy_command(
                    ctx=object(),
                    app_name="tracer_core_shell",
                    build_dir_name="build_tidy_core_family",
                    source_scope="core_family",
                )

        self.assertEqual(ret, 7)
        self.assertFalse(services.print_summary_called)
