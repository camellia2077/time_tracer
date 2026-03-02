from unittest.mock import patch

from .test_verify_fixtures import VerifyCommand, VerifyCommandTestBase, make_fake_build_command


class TestVerifyExecuteFlow(VerifyCommandTestBase):
    def test_execute_tracer_core_builds_tracer_windows_rust_cli_then_runs_suite(self):
        class FakeBuildCommand:
            build_calls = []

            def __init__(self, _ctx):
                pass

            def build(self, **kwargs):
                FakeBuildCommand.build_calls.append(kwargs)
                return 0

            def resolve_build_dir_name(self, **_kwargs):
                return "build_fast"

        with patch("toolchain.commands.cmd_quality.verify.BuildCommand", FakeBuildCommand):
            with patch(
                "toolchain.commands.cmd_quality.verify.run_command", return_value=0
            ) as mocked_run:
                result = self.command.execute(
                    app_name="tracer_core",
                    build_dir_name="build_fast",
                    concise=True,
                )

        self.assertEqual(result, 0)
        self.assertGreaterEqual(len(FakeBuildCommand.build_calls), 2)
        self.assertEqual(FakeBuildCommand.build_calls[0]["app_name"], "tracer_core")
        self.assertEqual(
            FakeBuildCommand.build_calls[-1]["app_name"],
            "tracer_windows_rust_cli",
        )
        called_cmd = None
        for call in mocked_run.call_args_list:
            cmd = call.args[0]
            if "--suite" in cmd and "artifact_windows_cli" in cmd:
                called_cmd = cmd
                break
        self.assertIsNotNone(called_cmd)
        self.assertIn("--suite", called_cmd)
        self.assertIn("artifact_windows_cli", called_cmd)
        self.assertNotIn("--with-build", called_cmd)
        self.assertNotIn("--skip-configure", called_cmd)
        self.assertIn("--concise", called_cmd)

    def test_execute_unmapped_app_writes_build_only_result_on_success(self):
        with (
            patch(
                "toolchain.commands.cmd_quality.verify.BuildCommand",
                make_fake_build_command(exit_code=0),
            ),
            patch.object(
                VerifyCommand,
                "_write_build_only_result_json",
                return_value=None,
            ) as mocked_writer,
        ):
            result = self.command.execute(
                app_name="unknown_app",
                build_dir_name="build_fast",
                concise=True,
            )

        self.assertEqual(result, 0)
        mocked_writer.assert_called_once()
        call_kwargs = mocked_writer.call_args.kwargs
        self.assertEqual(call_kwargs["app_name"], "unknown_app")
        self.assertEqual(call_kwargs["build_dir_name"], "build_fast")
        self.assertTrue(call_kwargs["success"])
        self.assertEqual(call_kwargs["exit_code"], 0)

    def test_execute_unmapped_app_writes_build_only_result_on_build_failure(self):
        with (
            patch(
                "toolchain.commands.cmd_quality.verify.BuildCommand",
                make_fake_build_command(exit_code=3),
            ),
            patch.object(
                VerifyCommand,
                "_write_build_only_result_json",
                return_value=None,
            ) as mocked_writer,
        ):
            result = self.command.execute(
                app_name="unknown_app",
                build_dir_name="build_fast",
                concise=True,
            )

        self.assertEqual(result, 3)
        mocked_writer.assert_called_once()
        call_kwargs = mocked_writer.call_args.kwargs
        self.assertEqual(call_kwargs["app_name"], "unknown_app")
        self.assertEqual(call_kwargs["build_dir_name"], "build_fast")
        self.assertFalse(call_kwargs["success"])
        self.assertEqual(call_kwargs["exit_code"], 3)

    def test_execute_mapped_app_writes_result_on_build_failure(self):
        with (
            patch(
                "toolchain.commands.cmd_quality.verify.BuildCommand",
                make_fake_build_command(exit_code=5),
            ),
            patch.object(
                VerifyCommand,
                "_write_build_only_result_json",
                return_value=None,
            ) as mocked_writer,
        ):
            result = self.command.execute(
                app_name="tracer_android",
                build_dir_name="build_fast",
                profile_name="android_style",
                concise=True,
            )

        self.assertEqual(result, 5)
        mocked_writer.assert_called_once()
        call_kwargs = mocked_writer.call_args.kwargs
        self.assertEqual(call_kwargs["app_name"], "tracer_android")
        self.assertEqual(call_kwargs["build_dir_name"], "build_fast")
        self.assertFalse(call_kwargs["success"])
        self.assertEqual(call_kwargs["exit_code"], 5)
        self.assertFalse(call_kwargs["build_only"])

    def test_execute_unit_scope_only_runs_unit_checks(self):
        with (
            patch(
                "toolchain.commands.cmd_quality.verify.BuildCommand",
                make_fake_build_command(exit_code=0),
            ),
            patch.object(
                VerifyCommand,
                "run_unit_scope_checks",
                return_value=0,
            ) as mocked_unit,
            patch.object(
                VerifyCommand,
                "run_artifact_scope_checks",
                return_value=0,
            ) as mocked_artifact,
        ):
            result = self.command.execute(
                app_name="tracer_core",
                build_dir_name="build_fast",
                verify_scope="unit",
            )

        self.assertEqual(result, 0)
        mocked_unit.assert_called_once()
        mocked_artifact.assert_not_called()

    def test_execute_artifact_scope_only_runs_artifact_checks(self):
        with (
            patch(
                "toolchain.commands.cmd_quality.verify.BuildCommand",
                make_fake_build_command(exit_code=0),
            ),
            patch.object(
                VerifyCommand,
                "run_unit_scope_checks",
                return_value=0,
            ) as mocked_unit,
            patch.object(
                VerifyCommand,
                "run_artifact_scope_checks",
                return_value=0,
            ) as mocked_artifact,
        ):
            result = self.command.execute(
                app_name="tracer_core",
                build_dir_name="build_fast",
                verify_scope="artifact",
            )

        self.assertEqual(result, 0)
        mocked_unit.assert_not_called()
        mocked_artifact.assert_called_once()

    def test_execute_batch_scope_runs_unit_then_artifact(self):
        with (
            patch(
                "toolchain.commands.cmd_quality.verify.BuildCommand",
                make_fake_build_command(exit_code=0),
            ),
            patch.object(
                VerifyCommand,
                "run_unit_scope_checks",
                return_value=0,
            ) as mocked_unit,
            patch.object(
                VerifyCommand,
                "run_artifact_scope_checks",
                return_value=0,
            ) as mocked_artifact,
        ):
            result = self.command.execute(
                app_name="tracer_core",
                build_dir_name="build_fast",
                verify_scope="batch",
            )

        self.assertEqual(result, 0)
        mocked_unit.assert_called_once()
        mocked_artifact.assert_called_once()
