import sys
from unittest import TestCase
from unittest.mock import patch

from .test_verify_fixtures import Context, REPO_ROOT
from tools.toolchain.commands.cmd_quality.self_test import SelfTestCommand


class TestSelfTestCommand(TestCase):
    @classmethod
    def setUpClass(cls):
        cls.ctx = Context(REPO_ROOT)

    def test_execute_verify_stack_group_dispatches_named_subset(self):
        command = SelfTestCommand(self.ctx)

        with patch(
            "tools.toolchain.commands.cmd_quality.self_test.run_command",
            return_value=0,
        ) as mocked_run:
            result = command.execute(group="verify-stack", verbose=False)

        self.assertEqual(result, 0)
        called_cmd = mocked_run.call_args.args[0]
        self.assertEqual(called_cmd[:3], [sys.executable, "-m", "unittest"])
        self.assertIn("tools.tests.verify.test_verify_execute_flow", called_cmd)
        self.assertIn("tools.tests.validate.test_validate_command", called_cmd)
        self.assertIn("tools.tests.platform.test_tidy_step", called_cmd)
        self.assertIn("tools.tests.run_cli.test_run_cli_dispatch", called_cmd)
        self.assertNotIn("discover", called_cmd)
        self.assertNotIn("-v", called_cmd)

    def test_execute_default_uses_discovery(self):
        command = SelfTestCommand(self.ctx)

        with patch(
            "tools.toolchain.commands.cmd_quality.self_test.run_command",
            return_value=0,
        ) as mocked_run:
            result = command.execute(verbose=True)

        self.assertEqual(result, 0)
        called_cmd = mocked_run.call_args.args[0]
        self.assertEqual(called_cmd[:4], [sys.executable, "-m", "unittest", "discover"])
        self.assertIn("-v", called_cmd)
