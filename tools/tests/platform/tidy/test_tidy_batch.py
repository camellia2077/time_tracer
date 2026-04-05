import json
import io
from pathlib import Path
from tempfile import TemporaryDirectory
from types import SimpleNamespace
from unittest import TestCase
from unittest.mock import patch
from contextlib import redirect_stdout

from tools.toolchain.commands.tidy.batch import TidyBatchCommand
from tools.toolchain.commands.tidy.batch_internal.tidy_batch_pipeline import run_refresh_stage
from tools.toolchain.commands.tidy.workspace import ResolvedTidyWorkspace
from tools.toolchain.core.context import Context

REPO_ROOT = Path(__file__).resolve().parents[4]


class TestTidyBatchCommand(TestCase):
    def test_execute_writes_refresh_running_result_before_long_refresh(self):
        with TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            batch_dir = root / "tasks" / "batch_001"
            batch_dir.mkdir(parents=True)
            (batch_dir / "task_001.json").write_text(
                json.dumps(
                    {
                        "version": 2,
                        "task_id": "001",
                        "batch_id": "batch_001",
                        "queue_batch_id": "batch_001",
                        "source_file": "C:/code/time_tracer/libs/tracer_core/src/demo.cpp",
                        "compiler_errors": False,
                        "checks": ["google-runtime-int"],
                        "diagnostics": [
                            {
                                "line": 3,
                                "col": 1,
                                "severity": "warning",
                                "check": "google-runtime-int",
                                "message": "consider replacing 'long long' with 'int64'",
                            }
                        ],
                    },
                    indent=2,
                ),
                encoding="utf-8",
            )

            ctx = Context(REPO_ROOT)
            ctx.get_tidy_layout = lambda *_args, **_kwargs: SimpleNamespace(
                tasks_dir=root / "tasks",
                tasks_done_dir=root / "tasks_done",
                tidy_result_path=root / "tidy_result.json",
                batch_state_path=root / "batch_state.json",
            )
            workspace = ResolvedTidyWorkspace(
                source_scope="core_family",
                build_dir_name="build_tidy_core_family",
                source_roots=[],
                prebuild_targets=[],
            )

            statuses: list[str] = []

            def _record_result(**kwargs):
                statuses.append(str(kwargs["status"]))
                return root / "tidy_result.json"

            with (
                patch(
                    "tools.toolchain.commands.tidy.batch.tidy_workspace.resolve_workspace",
                    return_value=workspace,
                ),
                patch(
                    "tools.toolchain.commands.tidy.batch.tidy_result_summary.write_tidy_result",
                    side_effect=_record_result,
                ),
                patch(
                    "tools.toolchain.commands.tidy.batch.load_checkpoint",
                    return_value=None,
                ),
                patch(
                    "tools.toolchain.commands.tidy.batch.save_checkpoint",
                    return_value=None,
                ),
                patch(
                    "tools.toolchain.commands.tidy.batch.CleanCommand.execute",
                    return_value=0,
                ),
                patch(
                    "tools.toolchain.commands.tidy.batch.run_refresh_stage",
                    return_value=124,
                ),
            ):
                ret = TidyBatchCommand(ctx).execute(
                    app_name="tracer_core_shell",
                    batch_id="001",
                    source_scope="core_family",
                    tidy_build_dir_name="build_tidy_core_family",
                )

            self.assertEqual(ret, 124)
            self.assertIn("refresh_running", statuses)
            self.assertIn("timeout_during_refresh", statuses)

    def test_execute_records_post_refresh_queue_state_for_reresolve(self):
        with TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            batch_one = root / "tasks" / "batch_001"
            batch_two = root / "tasks" / "batch_002"
            batch_one.mkdir(parents=True)
            batch_two.mkdir(parents=True)
            (batch_one / "task_001.json").write_text(
                json.dumps(
                    {
                        "version": 2,
                        "task_id": "001",
                        "batch_id": "batch_001",
                        "queue_batch_id": "batch_001",
                        "source_file": "C:/code/time_tracer/libs/tracer_core/src/current.cpp",
                        "compiler_errors": False,
                        "checks": ["google-runtime-int"],
                        "diagnostics": [
                            {
                                "line": 3,
                                "col": 1,
                                "severity": "warning",
                                "check": "google-runtime-int",
                                "message": "consider replacing 'long long' with 'int64'",
                            }
                        ],
                    },
                    indent=2,
                ),
                encoding="utf-8",
            )
            (batch_two / "task_011.json").write_text(
                json.dumps(
                    {
                        "version": 2,
                        "task_id": "011",
                        "batch_id": "batch_002",
                        "queue_batch_id": "batch_002",
                        "source_file": "C:/code/time_tracer/libs/tracer_core/src/next.cpp",
                        "compiler_errors": False,
                        "checks": ["google-runtime-int"],
                        "diagnostics": [
                            {
                                "line": 8,
                                "col": 2,
                                "severity": "warning",
                                "check": "google-runtime-int",
                                "message": "consider replacing 'long long' with 'int64'",
                            }
                        ],
                    },
                    indent=2,
                ),
                encoding="utf-8",
            )

            ctx = Context(REPO_ROOT)
            ctx.get_tidy_layout = lambda *_args, **_kwargs: SimpleNamespace(
                tasks_dir=root / "tasks",
                tasks_done_dir=root / "tasks_done",
                tidy_result_path=root / "tidy_result.json",
                batch_state_path=root / "batch_state.json",
            )
            workspace = ResolvedTidyWorkspace(
                source_scope="core_family",
                build_dir_name="build_tidy_core_family",
                source_roots=[],
                prebuild_targets=[],
            )

            recorded_update_kwargs = {}

            def _record_update_state(**kwargs):
                recorded_update_kwargs.update(kwargs)
                return root / "batch_state.json"

            with (
                patch(
                    "tools.toolchain.commands.tidy.batch.tidy_workspace.resolve_workspace",
                    return_value=workspace,
                ),
                patch(
                    "tools.toolchain.commands.tidy.batch.tidy_result_summary.write_tidy_result",
                    return_value=root / "tidy_result.json",
                ),
                patch(
                    "tools.toolchain.commands.tidy.batch.load_checkpoint",
                    return_value=None,
                ),
                patch(
                    "tools.toolchain.commands.tidy.batch.save_checkpoint",
                    return_value=None,
                ),
                patch(
                    "tools.toolchain.commands.tidy.batch.CleanCommand.execute",
                    side_effect=lambda **_kwargs: (batch_one / "task_001.json").unlink() or 0,
                ),
                patch(
                    "tools.toolchain.commands.tidy.batch.run_refresh_stage",
                    return_value=0,
                ),
                patch(
                    "tools.toolchain.commands.tidy.batch.batch_state.update_state",
                    side_effect=_record_update_state,
                ),
            ):
                ret = TidyBatchCommand(ctx).execute(
                    app_name="tracer_core_shell",
                    batch_id="001",
                    source_scope="core_family",
                    tidy_build_dir_name="build_tidy_core_family",
                )

            self.assertEqual(ret, 0)
            extra_fields = recorded_update_kwargs["extra_fields"]
            self.assertTrue(extra_fields["queue_requires_reresolve"])
            self.assertTrue(extra_fields["historical_selection_stale"])
            self.assertEqual(extra_fields["reparse_required_reason"], "refresh_completed")
            self.assertEqual(extra_fields["next_queue_head"]["task_id"], "011")
            self.assertEqual(extra_fields["replacement_queue_head"]["task_id"], "011")
            self.assertEqual(extra_fields["historical_batch"]["batch_id"], "batch_001")
            self.assertTrue(extra_fields["historical_batch"]["historical_identity_stale"])
            self.assertTrue(extra_fields["historical_batch"]["queue_identity_changed_after_refresh"])
            self.assertIn("Refresh replaced closed selection", extra_fields["queue_transition_summary"])
            self.assertIn("historical batch/task selection is now stale", extra_fields["next_action"])
            self.assertIn("re-resolve the current smallest pending task", extra_fields["next_action"])

    def test_run_refresh_stage_emits_heartbeat_for_long_running_refresh(self):
        class FakeProcess:
            def __init__(self):
                self.returncode = None

            def poll(self):
                if clock[0] >= 31:
                    return 0
                return None

            def terminate(self):
                self.returncode = -1

            def wait(self, timeout=None):
                return self.returncode if self.returncode is not None else 0

            def kill(self):
                self.returncode = -9

        clock = [0.0]

        def _fake_monotonic():
            return clock[0]

        def _fake_sleep(seconds):
            clock[0] += float(seconds)

        ctx = SimpleNamespace(
            repo_root=REPO_ROOT,
            setup_env=lambda: {},
        )
        stdout = io.StringIO()

        with (
            patch(
                "tools.toolchain.commands.tidy.batch_internal.tidy_batch_pipeline.subprocess.Popen",
                return_value=FakeProcess(),
            ),
            patch(
                "tools.toolchain.commands.tidy.batch_internal.tidy_batch_pipeline.time.monotonic",
                side_effect=_fake_monotonic,
            ),
            patch(
                "tools.toolchain.commands.tidy.batch_internal.tidy_batch_pipeline.time.sleep",
                side_effect=_fake_sleep,
            ),
            redirect_stdout(stdout),
        ):
            ret = run_refresh_stage(
                ctx=ctx,
                refresh_command_cls=None,
                app_name="tracer_core_shell",
                batch_id="batch_001",
                full_every=3,
                keep_going=True,
                jobs=None,
                parse_workers=None,
                concise=False,
                source_scope="core_family",
                tidy_build_dir_name="build_tidy_core_family",
                start_time=0.0,
                timeout_seconds=None,
            )

        self.assertEqual(ret, 0)
        self.assertIn("refresh still running for batch_001", stdout.getvalue())

    def test_run_refresh_stage_forwards_strict_config_flag(self):
        class FakeProcess:
            def poll(self):
                return 0

        ctx = SimpleNamespace(
            repo_root=REPO_ROOT,
            setup_env=lambda: {},
        )
        spawned_commands: list[list[str]] = []

        def _capture_popen(command, **_kwargs):
            spawned_commands.append(command)
            return FakeProcess()

        with patch(
            "tools.toolchain.commands.tidy.batch_internal.tidy_batch_pipeline.subprocess.Popen",
            side_effect=_capture_popen,
        ):
            ret = run_refresh_stage(
                ctx=ctx,
                refresh_command_cls=None,
                app_name="tracer_core_shell",
                batch_id="batch_001",
                full_every=3,
                keep_going=True,
                jobs=5,
                parse_workers=7,
                concise=True,
                source_scope="core_family",
                tidy_build_dir_name="build_tidy_core_family",
                start_time=0.0,
                timeout_seconds=None,
                strict_config=True,
            )

        self.assertEqual(ret, 0)
        self.assertEqual(len(spawned_commands), 1)
        self.assertIn("--jobs", spawned_commands[0])
        self.assertIn("5", spawned_commands[0])
        self.assertIn("--parse-workers", spawned_commands[0])
        self.assertIn("7", spawned_commands[0])
        self.assertIn("--concise", spawned_commands[0])
        self.assertIn("--strict-config", spawned_commands[0])
