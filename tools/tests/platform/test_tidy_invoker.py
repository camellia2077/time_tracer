from pathlib import Path
from unittest import TestCase
from unittest.mock import patch

from tools.toolchain.commands.tidy import invoker as tidy_invoker
from tools.toolchain.core.context import Context

REPO_ROOT = Path(__file__).resolve().parents[3]


class TestTidyInvokerJobs(TestCase):
    def test_resolve_effective_tidy_jobs_uses_bounded_full_auto_throttle(self):
        ctx = Context(REPO_ROOT)
        ctx.config.tidy.jobs_full = 0

        with patch("tools.toolchain.commands.tidy.invoker.os.cpu_count", return_value=24):
            effective_jobs = tidy_invoker.resolve_effective_tidy_jobs(
                ctx,
                None,
                mode="full",
            )

        self.assertEqual(effective_jobs, 6)

    def test_resolve_effective_tidy_jobs_uses_separate_task_batch_lane(self):
        ctx = Context(REPO_ROOT)
        ctx.config.tidy.jobs_task_batch = 0

        with patch("tools.toolchain.commands.tidy.invoker.os.cpu_count", return_value=24):
            effective_jobs = tidy_invoker.resolve_effective_tidy_jobs(
                ctx,
                None,
                mode="task_batch",
            )

        self.assertEqual(effective_jobs, 8)

    def test_resolve_effective_tidy_jobs_honors_explicit_override(self):
        ctx = Context(REPO_ROOT)
        ctx.config.tidy.jobs_full = 0
        ctx.config.tidy.jobs_task_batch = 0

        self.assertEqual(
            tidy_invoker.resolve_effective_tidy_jobs(ctx, 3, mode="full"),
            3,
        )
        self.assertEqual(
            tidy_invoker.resolve_effective_tidy_jobs(ctx, 5, mode="task_batch"),
            5,
        )
