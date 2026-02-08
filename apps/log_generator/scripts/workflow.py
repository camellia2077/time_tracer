#!/usr/bin/env python3
from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
sys.path.append(str(SCRIPT_DIR))

from workflow.analyze_tasks import analyze_tasks
from workflow.log_analyzer import (
    analyze_task_batch,
    analyze_tasks as analyze_tasks_summary,
    generate_markdown_summary,
    print_batch_summary,
    save_json_summary,
)
from workflow.task_manager import cleanup_task_logs, list_tasks
import workflow.verify_batch as verify_batch
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


def _cmd_analyze(args: argparse.Namespace) -> int:
    tasks_dir = _resolve_tasks_dir(args.dir)
    analyze_tasks(args.start, args.end, str(tasks_dir))
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


def _cmd_summary(args: argparse.Namespace) -> int:
    tasks_dir = _resolve_tasks_dir(args.dir)
    summary = analyze_tasks_summary(tasks_dir)
    if not summary:
        print("No tasks found.")
        return 0
    save_json_summary(summary, tasks_dir / "tasks_summary.json")
    generate_markdown_summary(summary, tasks_dir / "tasks_summary.md", SCRIPT_DIR.parent)
    print(f"--- Wrote task summaries to {tasks_dir}")
    return 0


def _cmd_split(args: argparse.Namespace) -> int:
    log_path = Path(args.log) if args.log else (SCRIPT_DIR.parent / "build_tidy" / "build.log")
    out_dir = Path(args.out) if args.out else (SCRIPT_DIR.parent / "build_tidy" / "tasks")

    if not log_path.exists():
        print(f"Log not found: {log_path}")
        return 1

    lines = log_path.read_text(encoding="utf-8", errors="replace").splitlines(keepends=True)
    count = split_tidy_logs(lines, out_dir)
    print(f"--- Created {count} task log files in {out_dir}")
    return 0


def _cmd_batch(args: argparse.Namespace) -> int:
    tasks_dir = _resolve_tasks_dir(args.dir)
    summary = analyze_task_batch(tasks_dir, args.task_ids)
    print_batch_summary(summary)
    return 0


def _cmd_verify(args: argparse.Namespace) -> int:
    original_argv = sys.argv
    try:
        sys.argv = ["verify_batch.py"] + args.task_ids
        verify_batch.main()
    finally:
        sys.argv = original_argv
    return 0


def _cmd_tidy(args: argparse.Namespace) -> int:
    project_dir = SCRIPT_DIR.parent
    build_dir = project_dir / "build_tidy"
    tasks_dir = build_dir / "tasks"
    log_path = build_dir / "build.log"

    build_dir.mkdir(parents=True, exist_ok=True)

    build_script = SCRIPT_DIR / "build.py"
    cmd = [sys.executable, str(build_script), "tidy"]
    cmd.extend(args.extra_args)

    log_path.parent.mkdir(parents=True, exist_ok=True)
    with log_path.open("w", encoding="utf-8") as handle:
        process = subprocess.Popen(
            cmd,
            cwd=project_dir,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            encoding="utf-8",
            errors="replace",
        )
        for line in process.stdout:
            handle.write(line)
            print(line, end="")
        return_code = process.wait()

    if log_path.exists():
        lines = log_path.read_text(encoding="utf-8", errors="replace").splitlines(keepends=True)
        count = split_tidy_logs(lines, tasks_dir)
        print(f"--- Created {count} task log files in {tasks_dir}")

        summary = analyze_tasks_summary(tasks_dir)
        if summary:
            save_json_summary(summary, tasks_dir / "tasks_summary.json")
            generate_markdown_summary(summary, tasks_dir / "tasks_summary.md", project_dir)
            print(f"--- Wrote task summaries to {tasks_dir}")
    else:
        print(f"Log not found: {log_path}")

    return return_code


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Workflow CLI for log_generator.")
    subparsers = parser.add_subparsers(dest="command", required=True)

    analyze_parser = subparsers.add_parser("analyze", help="Analyze task log range.")
    analyze_parser.add_argument("start", type=int, help="Start Task ID")
    analyze_parser.add_argument("end", type=int, help="End Task ID")
    analyze_parser.add_argument("--dir", help="Tasks directory path")
    analyze_parser.set_defaults(func=_cmd_analyze)

    list_parser = subparsers.add_parser("list", help="List available task logs.")
    list_parser.add_argument("--dir", help="Tasks directory path")
    list_parser.set_defaults(func=_cmd_list)

    clean_parser = subparsers.add_parser("clean", help="Clean up task logs.")
    clean_parser.add_argument("task_ids", nargs="+", help="Task IDs to remove")
    clean_parser.add_argument("--dir", help="Tasks directory path")
    clean_parser.set_defaults(func=_cmd_clean)

    summary_parser = subparsers.add_parser("summary", help="Generate tasks_summary.md/json.")
    summary_parser.add_argument("--dir", help="Tasks directory path")
    summary_parser.set_defaults(func=_cmd_summary)

    split_parser = subparsers.add_parser("split", help="Split build.log into task_XXX.log files.")
    split_parser.add_argument("--log", help="Path to build.log")
    split_parser.add_argument("--out", help="Directory for task_XXX.log files")
    split_parser.set_defaults(func=_cmd_split)

    batch_parser = subparsers.add_parser("batch", help="Print batch summary for task IDs.")
    batch_parser.add_argument("task_ids", nargs="+", help="Task IDs to summarize")
    batch_parser.add_argument("--dir", help="Tasks directory path")
    batch_parser.set_defaults(func=_cmd_batch)

    verify_parser = subparsers.add_parser("verify", help="Verify tasks and cleanup.")
    verify_parser.add_argument("task_ids", nargs="+", help="Task IDs (or Start End range)")
    verify_parser.set_defaults(func=_cmd_verify)

    tidy_parser = subparsers.add_parser("tidy", help="Run tidy, split logs, and write summaries.")
    tidy_parser.add_argument("extra_args", nargs=argparse.REMAINDER, help="Extra args passed to build.py")
    tidy_parser.set_defaults(func=_cmd_tidy)

    args = parser.parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    raise SystemExit(main())
