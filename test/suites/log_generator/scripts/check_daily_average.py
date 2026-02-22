#!/usr/bin/env python3
from __future__ import annotations

import argparse
import sys
from pathlib import Path

from check_daily_average_lib import (
    MonthStats,
    ParsedDay,
    ParsedEvent,
    calculate_average_minutes,
    calculate_day_minutes,
    collect_month_files,
    diff_wrap_minutes,
    extract_description,
    is_day_marker,
    is_event_line,
    load_wake_keywords,
    parse_minute_of_day,
    parse_month_file,
)

__all__ = [
    "MonthStats",
    "ParsedDay",
    "ParsedEvent",
    "calculate_average_minutes",
    "calculate_day_minutes",
    "collect_month_files",
    "diff_wrap_minutes",
    "extract_description",
    "is_day_marker",
    "is_event_line",
    "load_wake_keywords",
    "main",
    "parse_args",
    "parse_minute_of_day",
    "parse_month_file",
]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Check generated log daily average duration stays within bounds."
    )
    parser.add_argument("--input-root", required=True, help="Root directory of logs.")
    parser.add_argument(
        "--activities-config",
        required=True,
        help="Path to activities_config.toml used to load wake keywords.",
    )
    parser.add_argument("--min-hours", type=float, required=True)
    parser.add_argument("--max-hours", type=float, required=True)
    parser.add_argument("--label", default="dataset")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    input_root = Path(args.input_root).resolve()
    activities_config_path = Path(args.activities_config).resolve()

    if not input_root.exists():
        print(f"[FAIL] {args.label}: input root does not exist: {input_root}")
        return 1
    if not activities_config_path.exists():
        print(f"[FAIL] {args.label}: activities config does not exist: {activities_config_path}")
        return 1
    if args.max_hours < args.min_hours:
        print(f"[FAIL] {args.label}: invalid range min={args.min_hours} max={args.max_hours}")
        return 1

    wake_keywords = load_wake_keywords(activities_config_path)
    month_files = collect_month_files(input_root)
    if not month_files:
        print(f"[FAIL] {args.label}: no month files found under {input_root}")
        return 1

    average_minutes, total_days, monthly_stats = calculate_average_minutes(
        month_files, wake_keywords
    )
    average_hours = average_minutes / 60.0

    min_month = min(monthly_stats, key=lambda item: item.average_minutes)
    max_month = max(monthly_stats, key=lambda item: item.average_minutes)

    print(
        f"[INFO] {args.label}: files={len(month_files)}, days={total_days}, "
        f"avg={average_hours:.2f} h/day ({average_minutes:.2f} min/day)"
    )
    print(
        f"[INFO] {args.label}: min_month={min_month.year}-{min_month.month:02d} "
        f"({min_month.average_minutes / 60.0:.2f} h/day), "
        f"max_month={max_month.year}-{max_month.month:02d} "
        f"({max_month.average_minutes / 60.0:.2f} h/day)"
    )

    if args.min_hours <= average_hours <= args.max_hours:
        print(
            f"[PASS] {args.label}: average within [{args.min_hours:.2f}, "
            f"{args.max_hours:.2f}] h/day"
        )
        return 0

    print(
        f"[FAIL] {args.label}: average {average_hours:.2f} h/day not in "
        f"[{args.min_hours:.2f}, {args.max_hours:.2f}] h/day"
    )
    return 1


if __name__ == "__main__":
    sys.exit(main())
