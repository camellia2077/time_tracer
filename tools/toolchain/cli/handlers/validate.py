import argparse

from ...commands.cmd_validate import ValidateCommand
from ...core.context import Context
from ..model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, _defaults: ParserDefaults) -> None:
    parser.add_argument(
        "--plan",
        required=True,
        help="Path to the validation TOML plan file.",
    )
    parser.add_argument(
        "--paths",
        action="append",
        nargs="+",
        default=[],
        metavar="PATH",
        help="Changed or focused paths for this validation run. Can be repeated.",
    )
    parser.add_argument(
        "--paths-file",
        default=None,
        help="Text file with one focused path per line.",
    )
    parser.add_argument(
        "--run-name",
        default=None,
        help="Override the run name declared in the plan.",
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Mirror full command output to the console while still writing logs.",
    )


def run(args: argparse.Namespace, ctx: Context) -> int:
    raw_paths: list[str] = []
    for chunk in getattr(args, "paths", []) or []:
        raw_paths.extend(chunk)
    cmd = ValidateCommand(ctx)
    return cmd.execute(
        plan_path=args.plan,
        raw_paths=raw_paths,
        paths_file=args.paths_file,
        run_name=args.run_name,
        verbose=bool(args.verbose),
    )


COMMAND = CommandSpec(
    name="validate",
    help="Run a multi-track validation plan from TOML.",
    register=register,
    run=run,
    app_mode="none",
    add_app_path=False,
)
