import argparse

from ...commands.cmd_rename import RenameCommand
from ...core.context import Context
from ..model import CommandSpec, ParserDefaults


def _run_plan(args: argparse.Namespace, ctx: Context) -> int:
    cmd = RenameCommand(ctx)
    return cmd.plan(args.app, max_candidates=args.max_candidates, run_tidy=args.run_tidy)


def _register_plan(parser: argparse.ArgumentParser, _: ParserDefaults) -> None:
    parser.add_argument("--max-candidates", type=int, default=None)
    parser.add_argument("--run-tidy", action="store_true")


RENAME_PLAN_COMMAND = CommandSpec(
    name="rename-plan",
    register=_register_plan,
    run=_run_plan,
)


def _run_apply(args: argparse.Namespace, ctx: Context) -> int:
    cmd = RenameCommand(ctx)
    return cmd.apply(args.app, limit=args.limit, dry_run=args.dry_run, strict=args.strict)


def _register_apply(parser: argparse.ArgumentParser, _: ParserDefaults) -> None:
    parser.add_argument("--limit", type=int, default=0)
    parser.add_argument("--dry-run", action="store_true")
    parser.add_argument("--strict", action="store_true")


RENAME_APPLY_COMMAND = CommandSpec(
    name="rename-apply",
    register=_register_apply,
    run=_run_apply,
)


def _run_audit(args: argparse.Namespace, ctx: Context) -> int:
    cmd = RenameCommand(ctx)
    return cmd.audit(args.app, strict=args.strict)


def _register_audit(parser: argparse.ArgumentParser, _: ParserDefaults) -> None:
    parser.add_argument("--strict", action="store_true")


RENAME_AUDIT_COMMAND = CommandSpec(
    name="rename-audit",
    register=_register_audit,
    run=_run_audit,
)
