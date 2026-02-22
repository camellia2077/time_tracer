from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path

try:
    import tomllib
except ImportError:  # pragma: no cover
    import tomli as tomllib  # type: ignore


MONTH_FILE_PATTERN = re.compile(r"^(?P<year>\d{4})-(?P<month>\d{2})\.txt$")
EVENT_REMARK_DELIMITERS = ("//", "#", ";")
MINUTES_PER_DAY = 24 * 60
DEFAULT_WAKE_KEYWORD = "起床"


@dataclass
class ParsedEvent:
    minute_of_day: int
    description: str


@dataclass
class ParsedDay:
    getup_minute: int | None
    is_continuation: bool
    events: list[ParsedEvent]


@dataclass
class MonthStats:
    year: int
    month: int
    day_count: int
    total_minutes: int

    @property
    def average_minutes(self) -> float:
        if self.day_count == 0:
            return 0.0
        return self.total_minutes / self.day_count


def is_day_marker(line: str) -> bool:
    return len(line) == 4 and line.isdigit()


def is_event_line(line: str) -> bool:
    return len(line) >= 5 and line[:4].isdigit()


def parse_minute_of_day(hhmm: str) -> int:
    hour = int(hhmm[:2])
    minute = int(hhmm[2:4])
    return (hour * 60) + minute


def extract_description(event_tail: str) -> str:
    cutoff = len(event_tail)
    for delimiter in EVENT_REMARK_DELIMITERS:
        idx = event_tail.find(delimiter)
        if idx != -1 and idx < cutoff:
            cutoff = idx
    return event_tail[:cutoff].strip()


def diff_wrap_minutes(start_minute: int, end_minute: int) -> int:
    if end_minute < start_minute:
        end_minute += MINUTES_PER_DAY
    return end_minute - start_minute


def load_wake_keywords(path: Path) -> set[str]:
    data = tomllib.loads(path.read_text(encoding="utf-8"))
    raw_keywords = data.get("wake_keywords")
    if not isinstance(raw_keywords, list):
        return {DEFAULT_WAKE_KEYWORD}

    keywords = {str(keyword).strip() for keyword in raw_keywords if str(keyword).strip()}
    if not keywords:
        keywords.add(DEFAULT_WAKE_KEYWORD)
    return keywords


def parse_month_file(path: Path, wake_keywords: set[str]) -> list[ParsedDay]:
    days: list[ParsedDay] = []
    current_day: ParsedDay | None = None

    for raw_line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        line = raw_line.rstrip("\r").strip()
        if not line:
            continue

        if line.startswith("y") or line.startswith("m"):
            continue

        if is_day_marker(line):
            current_day = ParsedDay(getup_minute=None, is_continuation=False, events=[])
            days.append(current_day)
            continue

        if current_day is None or not is_event_line(line):
            continue

        minute_of_day = parse_minute_of_day(line[:4])
        description = extract_description(line[4:])
        is_wake_event = description in wake_keywords

        if is_wake_event:
            if current_day.getup_minute is None:
                current_day.getup_minute = minute_of_day
        elif current_day.getup_minute is None and not current_day.events:
            current_day.is_continuation = True

        current_day.events.append(
            ParsedEvent(minute_of_day=minute_of_day, description=description)
        )

    return days


def collect_month_files(root: Path) -> list[tuple[int, int, Path]]:
    month_files: list[tuple[int, int, Path]] = []
    for file_path in root.rglob("*.txt"):
        match = MONTH_FILE_PATTERN.match(file_path.name)
        if not match:
            continue
        year = int(match.group("year"))
        month = int(match.group("month"))
        month_files.append((year, month, file_path))

    month_files.sort(key=lambda item: (item[0], item[1], str(item[2])))
    return month_files


def calculate_day_minutes(
    day: ParsedDay,
    previous_day_last_minute: int | None,
    wake_keywords: set[str],
) -> tuple[int, int | None]:
    day_minutes = 0

    if (
        previous_day_last_minute is not None
        and day.getup_minute is not None
        and not day.is_continuation
    ):
        day_minutes += diff_wrap_minutes(previous_day_last_minute, day.getup_minute)

    current_start: int | None = None
    if day.is_continuation and previous_day_last_minute is not None:
        current_start = previous_day_last_minute
    elif day.getup_minute is not None:
        current_start = day.getup_minute

    for event in day.events:
        if event.description in wake_keywords:
            if current_start is None:
                current_start = event.minute_of_day
            continue

        if current_start is None:
            continue

        day_minutes += diff_wrap_minutes(current_start, event.minute_of_day)
        current_start = event.minute_of_day

    if day.events:
        previous_day_last_minute = day.events[-1].minute_of_day

    return day_minutes, previous_day_last_minute


def calculate_average_minutes(
    month_files: list[tuple[int, int, Path]],
    wake_keywords: set[str],
) -> tuple[float, int, list[MonthStats]]:
    total_minutes = 0
    total_days = 0
    monthly_stats: list[MonthStats] = []

    for year, month, file_path in month_files:
        days = parse_month_file(file_path, wake_keywords)
        month_minutes = 0
        previous_day_last_minute: int | None = None

        for day in days:
            day_minutes, previous_day_last_minute = calculate_day_minutes(
                day, previous_day_last_minute, wake_keywords
            )
            month_minutes += day_minutes

        month_day_count = len(days)
        total_minutes += month_minutes
        total_days += month_day_count
        monthly_stats.append(
            MonthStats(
                year=year,
                month=month,
                day_count=month_day_count,
                total_minutes=month_minutes,
            )
        )

    average_minutes = (total_minutes / total_days) if total_days > 0 else 0.0
    return average_minutes, total_days, monthly_stats
