from unittest.mock import patch

from ._run_cli_dispatch_test_support import RunCliDispatchTestBase


class TestRunCliDispatchTidyQueue(RunCliDispatchTestBase):
    def test_tidy_dispatches_source_scope_and_build_dir(self):
        class FakeTidyCommand:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, *args, **kwargs):
                FakeTidyCommand.last_kwargs = kwargs
                return 0

        with patch("tools.toolchain.cli.handlers.tidy.tidy.TidyCommand", FakeTidyCommand):
            self._assert_return_zero(
                [
                    "run.py", "tidy", "--app", "tracer_core_shell",
                    "--source-scope", "core_family",
                    "--build-dir", "build_tidy_core_family",
                ]
            )
        self.assertEqual(FakeTidyCommand.last_kwargs["source_scope"], "core_family")
        self.assertEqual(FakeTidyCommand.last_kwargs["build_dir_name"], "build_tidy_core_family")

    def test_tidy_refresh_has_no_batch_selector(self):
        class FakeRefresh:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeRefresh.last_kwargs = kwargs
                return 0

        with patch("tools.toolchain.cli.handlers.tidy.tidy_refresh.TidyRefreshCommand", FakeRefresh):
            self._assert_return_zero(
                ["run.py", "tidy-refresh", "--app", "tracer_core_shell", "--dry-run"]
            )
        self.assertTrue(FakeRefresh.last_kwargs["dry_run"])
        self.assertNotIn("batch_id", FakeRefresh.last_kwargs)

    def test_tidy_agent_dispatches_bounded_cluster_budget(self):
        class FakeAgent:
            last_kwargs = None

            def __init__(self, _ctx):
                pass

            def execute(self, **kwargs):
                FakeAgent.last_kwargs = kwargs
                return 0

        with patch("tools.toolchain.cli.handlers.tidy.tidy_agent.TidyAgentRunCommand", FakeAgent):
            self._assert_return_zero(
                [
                    "run.py", "tidy-agent", "--app", "tracer_core_shell",
                    "--max-clusters", "1", "--max-tasks", "5", "--max-minutes", "10",
                ]
            )
        self.assertEqual(FakeAgent.last_kwargs["max_clusters"], 1)
        self.assertEqual(FakeAgent.last_kwargs["max_tasks"], 5)
        self.assertEqual(FakeAgent.last_kwargs["max_minutes"], 10)
