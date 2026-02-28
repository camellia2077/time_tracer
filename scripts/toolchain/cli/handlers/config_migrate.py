import argparse

from ...commands.cmd_workflow.config_migrate import ConfigMigrateCommand
from ...core.context import Context
from ..model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, _: ParserDefaults) -> None:
    parser.add_argument(
        "--config-root",
        default=None,
        help=(
            "Override config root directory (default inferred from app; "
            "for tracer_windows_cli/tracer_core it is apps/tracer_core/config)."
        ),
    )
    parser.add_argument(
        "--profile",
        choices=["windows", "android", "shared"],
        default=None,
        help="Bundle profile value (default inferred from app).",
    )
    action_group = parser.add_mutually_exclusive_group()
    action_group.add_argument(
        "--apply",
        action="store_true",
        help="Write migrated bundle.toml and create bundle.toml.bak when updating.",
    )
    action_group.add_argument(
        "--rollback",
        action="store_true",
        help="Restore bundle.toml from bundle.toml.bak.",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Force dry-run mode (default behavior unless --apply or --rollback).",
    )
    parser.add_argument(
        "--show-diff",
        action="store_true",
        help="Print unified diff between existing and generated bundle.toml.",
    )


def run(args: argparse.Namespace, ctx: Context) -> int:
    cmd = ConfigMigrateCommand(ctx)
    effective_dry_run = bool(args.dry_run or (not args.apply and not args.rollback))
    return cmd.execute(
        app_name=args.app,
        config_root_arg=args.config_root,
        profile=args.profile,
        apply_changes=args.apply,
        dry_run=effective_dry_run,
        rollback=args.rollback,
        show_diff=args.show_diff,
    )


COMMAND = CommandSpec(
    name="config-migrate",
    register=register,
    run=run,
    app_mode="required",
    add_app_path=False,
    help=(
        "Migrate legacy config.toml path sections to meta/bundle.toml "
        "(idempotent, supports dry-run/apply/rollback)."
    ),
)
