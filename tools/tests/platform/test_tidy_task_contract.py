from pathlib import Path
from tempfile import TemporaryDirectory
from types import SimpleNamespace
from unittest import TestCase
from unittest.mock import patch

from tools.toolchain.commands.tidy.flow_internal.flow_prepare_phase import run_prepare_phase
from tools.toolchain.commands.tidy.flow_internal.flow_state import new_state
from tools.toolchain.commands.tidy.refresh_internal.refresh_runner import run_full_tidy
from tools.toolchain.core.context import Context

REPO_ROOT = Path(__file__).resolve().parents[3]


class TestTidyTaskContract(TestCase):
    def test_refresh_full_tidy_passes_task_view(self):
        class FakeTidyCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeTidyCommand.last_kwargs = kwargs
                return 0

        with patch(
            "tools.toolchain.commands.tidy.refresh_internal.refresh_runner.TidyCommand",
            FakeTidyCommand,
        ):
            ctx = Context(REPO_ROOT)
            ret = run_full_tidy(
                ctx=ctx,
                app_name="tracer_core_shell",
                jobs=8,
                parse_workers=4,
                keep_going=True,
                concise=True,
                source_scope="core_family",
                build_dir_name="build_tidy_core_family",
                task_view="toon",
            )

        self.assertEqual(ret, 0)
        self.assertEqual(FakeTidyCommand.last_kwargs["task_view"], "toon")
        self.assertTrue(FakeTidyCommand.last_kwargs["concise"])

    def test_flow_prepare_phase_passes_task_view(self):
        class FakeTidyCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeTidyCommand.last_kwargs = kwargs
                return 0

        with TemporaryDirectory() as temp_dir, patch(
            "tools.toolchain.commands.tidy.flow_internal.flow_prepare_phase.TidyCommand",
            FakeTidyCommand,
        ):
            ctx = Context(REPO_ROOT)
            options = SimpleNamespace(
                app_name="tracer_core_shell",
                resume=False,
                jobs=8,
                parse_workers=4,
                source_scope="core_family",
                tidy_build_dir_name="build_tidy_core_family",
                task_view="text+toon",
                config_file=None,
                strict_config=False,
            )
            state = new_state(
                app_name="tracer_core_shell",
                process_all=False,
                n=1,
                resume=False,
                test_every=3,
                concise=False,
                jobs=8,
                parse_workers=4,
                keep_going=True,
                run_tidy_fix=False,
                tidy_fix_limit=0,
                verify_build_dir="build_fast",
                profile_name=None,
                kill_build_procs=False,
                state_path=Path(temp_dir) / "flow_state.json",
            )
            ret = run_prepare_phase(
                ctx=ctx,
                options=options,
                state=state,
                tasks_dir=Path(temp_dir),
                effective_run_tidy_fix=False,
                effective_tidy_fix_limit=0,
                effective_keep_going=True,
            )

        self.assertEqual(ret, 0)
        self.assertEqual(FakeTidyCommand.last_kwargs["task_view"], "text+toon")
