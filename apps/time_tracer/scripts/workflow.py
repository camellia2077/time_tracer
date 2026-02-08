#!/usr/bin/env python3
from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
sys.path.append(str(SCRIPT_DIR))

from workflow.log_analyzer import analyze_tasks, generate_markdown_summary, save_json_summary
from workflow.task_manager import cleanup_task_logs, list_tasks
from workflow.task_splitter import split_tidy_logs


def _default_tasks_dir() -> Path:
    return SCRIPT_DIR.parent / "build_tidy" / "tasks"


def _resolve_tasks_dir(value: str | None) -> Path:
    if value is None:
        return _default_tasks_dir()
    path = Path(value)
    if not path.is_absolute():
        return (SCRIPT_DIR / value).resolve()
    return path


def _cmd_split(args: argparse.Namespace) -> int:
    log_path = Path(args.log) if args.log else (SCRIPT_DIR.parent / "build_tidy" / "build.log")
    out_dir = Path(args.out) if args.out else _default_tasks_dir()

    if not log_path.exists():
        print(f"Log not found: {log_path}")
        return 1

    lines = log_path.read_text(encoding="utf-8", errors="replace").splitlines()
    count = split_tidy_logs(lines, out_dir)
    print(f"--- Created {count} task log files in {out_dir}")
    return 0


def _cmd_summary(args: argparse.Namespace) -> int:
    tasks_dir = _resolve_tasks_dir(args.dir)
    summary = analyze_tasks(tasks_dir)
    if not summary:
        print("No tasks found.")
        return 0
    save_json_summary(summary, tasks_dir / "tasks_summary.json")
    generate_markdown_summary(summary, tasks_dir / "tasks_summary.md", SCRIPT_DIR.parent)
    print(f"--- Wrote task summaries to {tasks_dir}")
    return 0


def _cmd_list(args: argparse.Namespace) -> int:
    tasks_dir = _resolve_tasks_dir(args.dir)
    list_tasks(tasks_dir)
    return 0


def _cmd_clean(args: argparse.Namespace) -> int:
    tasks_dir = _resolve_tasks_dir(args.dir)
    count = cleanup_task_logs(tasks_dir, args.task_ids)
    print(f"Cleanup finished. Removed {count} log files.")
    return 0


def _cmd_tidy(args: argparse.Namespace) -> int:
    project_dir = SCRIPT_DIR.parent
    build_dir = project_dir / "build_tidy"
    tasks_dir = build_dir / "tasks"
    log_path = build_dir / "build.log"

    build_dir.mkdir(parents=True, exist_ok=True)

    build_script = SCRIPT_DIR / "build.py"
    cmd = [
        sys.executable,
        str(build_script),
        "--build-dir",
        "build_tidy",
        "--no-pch",
        "--tidy",
        "--fail-fast",
        "--analyze-tidy",
    ]
    cmd.extend(args.extra_args)

    result = subprocess.run(cmd, cwd=project_dir)

    if log_path.exists():
        lines = log_path.read_text(encoding="utf-8", errors="replace").splitlines()
        count = split_tidy_logs(lines, tasks_dir)
        print(f"--- Created {count} task log files in {tasks_dir}")

        summary = analyze_tasks(tasks_dir)
        if summary:
            save_json_summary(summary, tasks_dir / "tasks_summary.json")
            generate_markdown_summary(summary, tasks_dir / "tasks_summary.md", project_dir)
            print(f"--- Wrote task summaries to {tasks_dir}")
    else:
        print(f"Log not found: {log_path}")

    return result.returncode


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Workflow CLI for time_tracer.")
    subparsers = parser.add_subparsers(dest="command", required=True)

    split_parser = subparsers.add_parser("split", help="Split build.log into task_XXX.log files.")
    split_parser.add_argument("--log", help="Path to build.log")
    split_parser.add_argument("--out", help="Directory for task_XXX.log files")
    split_parser.set_defaults(func=_cmd_split)

    summary_parser = subparsers.add_parser("summary", help="Generate tasks_summary.md/json.")
    summary_parser.add_argument("--dir", help="Tasks directory path")
    summary_parser.set_defaults(func=_cmd_summary)

    list_parser = subparsers.add_parser("list", help="List available task logs.")
    list_parser.add_argument("--dir", help="Tasks directory path")
    list_parser.set_defaults(func=_cmd_list)

    clean_parser = subparsers.add_parser("clean", help="Clean up task logs.")
    clean_parser.add_argument("task_ids", nargs="+", help="Task IDs to remove")
    clean_parser.add_argument("--dir", help="Tasks directory path")
    clean_parser.set_defaults(func=_cmd_clean)

    tidy_parser = subparsers.add_parser("tidy", help="Run tidy, split logs, and write summaries.")
    tidy_parser.add_argument("extra_args", nargs=argparse.REMAINDER, help="Extra args passed to build.py")
    tidy_parser.set_defaults(func=_cmd_tidy)

    args = parser.parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
