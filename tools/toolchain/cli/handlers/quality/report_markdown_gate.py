import argparse

from ....commands.cmd_quality.report_markdown_gate import ReportMarkdownGateCommand
from ....core.context import Context
from ...common import add_build_dir_arg, reject_unsupported_build_dir_override
from ...model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, _defaults: ParserDefaults) -> None:
    add_build_dir_arg(
        parser,
        help_text=(
            "Build directory whose tracer_windows_rust_cli artifact output should be used "
            "(default: build_fast)."
        ),
    )
    parser.add_argument(
        "--refresh-golden",
        action="store_true",
        help="Sync current markdown cases into test/golden/report_markdown/v1 before re-auditing.",
    )
    parser.add_argument(
        "--cases-config",
        default=None,
        help=(
            "Optional gate case config TOML. Default: "
            "tools/suites/tracer_windows_rust_cli/tests/gate_cases.toml"
        ),
    )


def run(args: argparse.Namespace, ctx: Context) -> int:
    build_dir_error = reject_unsupported_build_dir_override(
        ctx=ctx,
        app_name=args.app,
        build_dir_name=args.build_dir,
        command_name="report-markdown-gate",
    )
    if build_dir_error != 0:
        return build_dir_error
    cmd = ReportMarkdownGateCommand(ctx)
    return cmd.execute(
        app_name=args.app,
        build_dir_name=args.build_dir,
        refresh_golden=bool(args.refresh_golden),
        cases_config_path=args.cases_config,
    )


COMMAND = CommandSpec(
    name="report-markdown-gate",
    register=register,
    run=run,
    help="Run the local markdown golden gate for artifact_windows_cli outputs.",
)
