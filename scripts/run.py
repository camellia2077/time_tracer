#!/usr/bin/env python3
import sys
import argparse
from pathlib import Path

# Add current directory to path
SCRIPT_DIR = Path(__file__).resolve().parent
sys.path.append(str(SCRIPT_DIR))

try:
    from agent.core.context import Context
    from agent.commands.build import BuildCommand
    from agent.commands.format import FormatCommand
    from agent.commands.tidy import TidyCommand
    from agent.commands.tidy_fix import TidyFixCommand
    from agent.commands.tidy_loop import TidyLoopCommand
    from agent.commands.tidy_flow import TidyFlowCommand
    from agent.commands.clean import CleanCommand
    from agent.commands.rename import RenameCommand
except ImportError as e:
    print(f"Error: Could not load internal agent modules.\n{e}")
    sys.exit(1)

def main():
    repo_root = SCRIPT_DIR.parent
    ctx = Context(repo_root)
    app_choices = list(ctx.config.apps.keys())

    parser = argparse.ArgumentParser(description="Unified Agent Toolchain")
    subparsers = parser.add_subparsers(dest="command", required=True)

    # Common args
    common = [
        "configure",
        "build",
        "format",
        "tidy",
        "tidy-split",
        "tidy-fix",
        "tidy-loop",
        "tidy-flow",
        "clean",
        "rename-plan",
        "rename-apply",
        "rename-audit",
    ]
    for cmd_name in common:
        p = subparsers.add_parser(cmd_name)
        # Handle dynamic choices if apps are defined, otherwise allow any string
        if app_choices:
            p.add_argument("--app", required=True, choices=app_choices)
        else:
            p.add_argument("--app", required=True)
        p.add_argument("--app-path", help="Override default application path")

    # Additional options
    subparsers.choices["configure"].add_argument("--tidy", action="store_true")
    subparsers.choices["configure"].add_argument(
        "--build-dir",
        default=None,
        help="Override build directory name (default: build_fast/build_tidy).",
    )
    subparsers.choices["configure"].add_argument(
        "--kill-build-procs",
        action="store_true",
        help="Kill cmake/ninja/ccache before configure/build (default: off)",
    )
    subparsers.choices["configure"].add_argument(
        "--no-kill-build-procs",
        action="store_true",
        help=argparse.SUPPRESS,
    )
    subparsers.choices["configure"].add_argument("extra_args", nargs=argparse.REMAINDER)
    
    subparsers.choices["build"].add_argument("--tidy", action="store_true")
    subparsers.choices["build"].add_argument(
        "--build-dir",
        default=None,
        help="Override build directory name (default: build_fast/build_tidy).",
    )
    subparsers.choices["build"].add_argument(
        "--kill-build-procs",
        action="store_true",
        help="Kill cmake/ninja/ccache before configure/build (default: off)",
    )
    subparsers.choices["build"].add_argument(
        "--no-kill-build-procs",
        action="store_true",
        help=argparse.SUPPRESS,
    )
    subparsers.choices["build"].add_argument("extra_args", nargs=argparse.REMAINDER)

    subparsers.choices["format"].add_argument("extra_args", nargs=argparse.REMAINDER)
    
    subparsers.choices["tidy"].add_argument("--jobs", type=int, default=None, help="Ninja parallel jobs, e.g. 16")
    subparsers.choices["tidy"].add_argument("--parse-workers", type=int, default=None, help="Parallel workers for log splitting")
    tidy_keep_going_group = subparsers.choices["tidy"].add_mutually_exclusive_group()
    tidy_keep_going_group.add_argument(
        "--keep-going",
        dest="keep_going",
        action="store_true",
        default=None,
        help="Enable Ninja keep-going mode (-k 0) for tidy build.",
    )
    tidy_keep_going_group.add_argument(
        "--no-keep-going",
        dest="keep_going",
        action="store_false",
        help="Disable Ninja keep-going mode (-k 0) for tidy build.",
    )
    subparsers.choices["tidy"].add_argument("extra_args", nargs=argparse.REMAINDER)

    subparsers.choices["tidy-split"].add_argument(
        "--parse-workers",
        type=int,
        default=None,
        help="Parallel workers for log splitting.",
    )
    subparsers.choices["tidy-split"].add_argument(
        "--max-lines",
        type=int,
        default=None,
        help="Max lines per generated task log (default from config).",
    )
    subparsers.choices["tidy-split"].add_argument(
        "--max-diags",
        type=int,
        default=None,
        help="Max diagnostics per generated task log (default from config).",
    )
    subparsers.choices["tidy-split"].add_argument(
        "--batch-size",
        type=int,
        default=None,
        help="Max task logs per batch folder (default from config).",
    )

    subparsers.choices["tidy-fix"].add_argument(
        "--limit",
        type=int,
        default=None,
        help="0/omit = full tidy-fix target; N>0 = tidy_fix_step_N (prefix range).",
    )
    subparsers.choices["tidy-fix"].add_argument(
        "--jobs",
        type=int,
        default=None,
        help="Ninja parallel jobs, e.g. 16",
    )
    tidy_fix_keep_going_group = subparsers.choices["tidy-fix"].add_mutually_exclusive_group()
    tidy_fix_keep_going_group.add_argument(
        "--keep-going",
        dest="keep_going",
        action="store_true",
        default=None,
        help="Enable Ninja keep-going mode (-k 0) for tidy-fix build.",
    )
    tidy_fix_keep_going_group.add_argument(
        "--no-keep-going",
        dest="keep_going",
        action="store_false",
        help="Disable Ninja keep-going mode (-k 0) for tidy-fix build.",
    )

    tidy_loop_target_group = subparsers.choices["tidy-loop"].add_mutually_exclusive_group()
    tidy_loop_target_group.add_argument("--n", type=int, default=None, help="Max tasks to clean in this run (default: 1)")
    tidy_loop_target_group.add_argument("--all", action="store_true", help="Process tasks until none remain (or manual task blocks)")
    subparsers.choices["tidy-loop"].add_argument("--test-every", type=int, default=1, help="Run verify after every K cleaned tasks")
    subparsers.choices["tidy-loop"].add_argument("--concise", action="store_true", help="Use concise test output when supported")
    subparsers.choices["tidy-loop"].add_argument(
        "--kill-build-procs",
        action="store_true",
        help="Kill cmake/ninja/ccache before loop verify builds (default: off)",
    )
    subparsers.choices["tidy-loop"].add_argument(
        "--no-kill-build-procs",
        action="store_true",
        help=argparse.SUPPRESS,
    )

    tidy_flow_target_group = subparsers.choices["tidy-flow"].add_mutually_exclusive_group()
    tidy_flow_target_group.add_argument("--n", type=int, default=None, help="Max tasks to process in this run (default: 1)")
    tidy_flow_target_group.add_argument("--all", action="store_true", help="Process tasks until none remain (or manual task blocks)")
    subparsers.choices["tidy-flow"].add_argument("--resume", action="store_true", help="Reuse existing task logs when present")
    subparsers.choices["tidy-flow"].add_argument("--test-every", type=int, default=3, help="Run verify after every K cleaned tasks inside tidy-loop")
    subparsers.choices["tidy-flow"].add_argument("--concise", action="store_true", help="Use concise test output when supported")
    subparsers.choices["tidy-flow"].add_argument("--jobs", type=int, default=None, help="Ninja parallel jobs for tidy generation, e.g. 16")
    subparsers.choices["tidy-flow"].add_argument("--parse-workers", type=int, default=None, help="Parallel workers for tidy log splitting")
    tidy_flow_keep_going_group = subparsers.choices["tidy-flow"].add_mutually_exclusive_group()
    tidy_flow_keep_going_group.add_argument(
        "--keep-going",
        dest="keep_going",
        action="store_true",
        default=None,
        help="Enable Ninja keep-going mode (-k 0) for tidy generation.",
    )
    tidy_flow_keep_going_group.add_argument(
        "--no-keep-going",
        dest="keep_going",
        action="store_false",
        help="Disable Ninja keep-going mode (-k 0) for tidy generation.",
    )
    tidy_flow_fix_group = subparsers.choices["tidy-flow"].add_mutually_exclusive_group()
    tidy_flow_fix_group.add_argument(
        "--with-tidy-fix",
        dest="run_tidy_fix",
        action="store_true",
        default=None,
        help="Run a tidy-fix pass before tidy task generation.",
    )
    tidy_flow_fix_group.add_argument(
        "--no-tidy-fix",
        dest="run_tidy_fix",
        action="store_false",
        help="Skip tidy-fix pass before tidy task generation.",
    )
    subparsers.choices["tidy-flow"].add_argument(
        "--tidy-fix-limit",
        type=int,
        default=None,
        help="0/omit = full tidy-fix target; N>0 = tidy_fix_step_N (prefix range).",
    )
    subparsers.choices["tidy-flow"].add_argument(
        "--kill-build-procs",
        action="store_true",
        help="Kill cmake/ninja/ccache before configure/build stages (default: off)",
    )
    subparsers.choices["tidy-flow"].add_argument(
        "--no-kill-build-procs",
        action="store_true",
        help=argparse.SUPPRESS,
    )
    
    subparsers.choices["clean"].add_argument("task_ids", nargs="+")
    
    subparsers.choices["rename-plan"].add_argument("--max-candidates", type=int, default=None)
    subparsers.choices["rename-plan"].add_argument("--run-tidy", action="store_true")
    
    subparsers.choices["rename-apply"].add_argument("--limit", type=int, default=0)
    subparsers.choices["rename-apply"].add_argument("--dry-run", action="store_true")
    subparsers.choices["rename-apply"].add_argument("--strict", action="store_true")
    
    subparsers.choices["rename-audit"].add_argument("--strict", action="store_true")

    args = parser.parse_args()
    
    if args.app_path:
        ctx.set_app_path_override(args.app, args.app_path)
    
    if args.command == "configure":
        kill_build_procs = bool(args.kill_build_procs and not args.no_kill_build_procs)
        cmd = BuildCommand(ctx)
        sys.exit(
            cmd.configure(
                args.app,
                args.tidy,
                args.extra_args,
                build_dir_name=args.build_dir,
                kill_build_procs=kill_build_procs,
            )
        )
    
    elif args.command == "build":
        kill_build_procs = bool(args.kill_build_procs and not args.no_kill_build_procs)
        cmd = BuildCommand(ctx)
        sys.exit(
            cmd.build(
                args.app,
                args.tidy,
                args.extra_args,
                build_dir_name=args.build_dir,
                kill_build_procs=kill_build_procs,
            )
        )

    elif args.command == "format":
        cmd = FormatCommand(ctx)
        sys.exit(cmd.execute(args.app, args.extra_args))
        
    elif args.command == "tidy":
        cmd = TidyCommand(ctx)
        sys.exit(
            cmd.execute(
                args.app,
                args.extra_args,
                jobs=args.jobs,
                parse_workers=args.parse_workers,
                keep_going=args.keep_going,
            )
        )

    elif args.command == "tidy-split":
        cmd = TidyCommand(ctx)
        sys.exit(
            cmd.split_only(
                app_name=args.app,
                parse_workers=args.parse_workers,
                max_lines=args.max_lines,
                max_diags=args.max_diags,
                batch_size=args.batch_size,
            )
        )

    elif args.command == "tidy-fix":
        cmd = TidyFixCommand(ctx)
        sys.exit(
            cmd.execute(
                app_name=args.app,
                limit=args.limit,
                jobs=args.jobs,
                keep_going=args.keep_going,
            )
        )

    elif args.command == "tidy-loop":
        kill_build_procs = bool(args.kill_build_procs and not args.no_kill_build_procs)
        effective_n = args.n if args.n is not None else 1
        cmd = TidyLoopCommand(ctx)
        sys.exit(
            cmd.execute(
                app_name=args.app,
                n=effective_n,
                process_all=args.all,
                test_every=args.test_every,
                concise=args.concise,
                kill_build_procs=kill_build_procs,
            )
        )

    elif args.command == "tidy-flow":
        kill_build_procs = bool(args.kill_build_procs and not args.no_kill_build_procs)
        effective_n = args.n if args.n is not None else 1
        cmd = TidyFlowCommand(ctx)
        sys.exit(
            cmd.execute(
                app_name=args.app,
                process_all=args.all,
                n=effective_n,
                resume=args.resume,
                test_every=args.test_every,
                concise=args.concise,
                jobs=args.jobs,
                parse_workers=args.parse_workers,
                keep_going=args.keep_going,
                run_tidy_fix=args.run_tidy_fix,
                tidy_fix_limit=args.tidy_fix_limit,
                kill_build_procs=kill_build_procs,
            )
        )
        
    elif args.command == "clean":
        cmd = CleanCommand(ctx)
        sys.exit(cmd.execute(args.app, args.task_ids))
    
    elif args.command == "rename-plan":
        cmd = RenameCommand(ctx)
        sys.exit(cmd.plan(args.app, max_candidates=args.max_candidates, run_tidy=args.run_tidy))
    
    elif args.command == "rename-apply":
        cmd = RenameCommand(ctx)
        sys.exit(cmd.apply(args.app, limit=args.limit, dry_run=args.dry_run, strict=args.strict))
    
    elif args.command == "rename-audit":
        cmd = RenameCommand(ctx)
        sys.exit(cmd.audit(args.app, strict=args.strict))

if __name__ == "__main__":
    main()

