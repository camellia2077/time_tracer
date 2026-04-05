from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

try:
    import tomllib
except ModuleNotFoundError:  # pragma: no cover
    import tomli as tomllib  # type: ignore


@dataclass(frozen=True)
class MarkdownFileCase:
    name: str
    source_relative_path: str


@dataclass(frozen=True)
class TripletFileCase:
    name_template: str
    source_relative_path_template: str


@dataclass(frozen=True)
class MarkdownCases:
    file_cases: tuple[MarkdownFileCase, ...]
    range_case_name: str
    range_argument: str
    recent_case_name: str
    recent_days: int
    recent_as_of: str


@dataclass(frozen=True)
class TripletCases:
    file_cases: tuple[TripletFileCase, ...]
    range_case_name_template: str
    range_argument: str


@dataclass(frozen=True)
class GateCasesConfig:
    markdown: MarkdownCases
    triplet: TripletCases


def _expect_dict(value, path: str) -> dict:
    if not isinstance(value, dict):
        raise ValueError(f"{path} must be a table.")
    return value


def _expect_str(value, path: str) -> str:
    if not isinstance(value, str) or not value.strip():
        raise ValueError(f"{path} must be a non-empty string.")
    return value.strip()


def _expect_positive_int(value, path: str) -> int:
    if not isinstance(value, int) or value <= 0:
        raise ValueError(f"{path} must be a positive integer.")
    return value


def _load_markdown_cases(markdown_payload: dict) -> MarkdownCases:
    raw_cases = markdown_payload.get("file_cases")
    if not isinstance(raw_cases, list) or not raw_cases:
        raise ValueError("[markdown].file_cases must be a non-empty array of tables.")

    file_cases: list[MarkdownFileCase] = []
    for index, item in enumerate(raw_cases):
        item_path = f"[markdown].file_cases[{index}]"
        item_dict = _expect_dict(item, item_path)
        file_cases.append(
            MarkdownFileCase(
                name=_expect_str(item_dict.get("name"), f"{item_path}.name"),
                source_relative_path=_expect_str(
                    item_dict.get("source_relative_path"),
                    f"{item_path}.source_relative_path",
                ),
            )
        )

    return MarkdownCases(
        file_cases=tuple(file_cases),
        range_case_name=_expect_str(
            markdown_payload.get("range_case_name"),
            "[markdown].range_case_name",
        ),
        range_argument=_expect_str(
            markdown_payload.get("range_argument"),
            "[markdown].range_argument",
        ),
        recent_case_name=_expect_str(
            markdown_payload.get("recent_case_name"),
            "[markdown].recent_case_name",
        ),
        recent_days=_expect_positive_int(
            markdown_payload.get("recent_days"),
            "[markdown].recent_days",
        ),
        recent_as_of=_expect_str(
            markdown_payload.get("recent_as_of"),
            "[markdown].recent_as_of",
        ),
    )


def _load_triplet_cases(triplet_payload: dict) -> TripletCases:
    raw_cases = triplet_payload.get("file_cases")
    if not isinstance(raw_cases, list) or not raw_cases:
        raise ValueError("[triplet].file_cases must be a non-empty array of tables.")

    file_cases: list[TripletFileCase] = []
    for index, item in enumerate(raw_cases):
        item_path = f"[triplet].file_cases[{index}]"
        item_dict = _expect_dict(item, item_path)
        file_cases.append(
            TripletFileCase(
                name_template=_expect_str(
                    item_dict.get("name_template"), f"{item_path}.name_template"
                ),
                source_relative_path_template=_expect_str(
                    item_dict.get("source_relative_path_template"),
                    f"{item_path}.source_relative_path_template",
                ),
            )
        )

    return TripletCases(
        file_cases=tuple(file_cases),
        range_case_name_template=_expect_str(
            triplet_payload.get("range_case_name_template"),
            "[triplet].range_case_name_template",
        ),
        range_argument=_expect_str(
            triplet_payload.get("range_argument"),
            "[triplet].range_argument",
        ),
    )


def load_gate_cases_config(path: Path) -> GateCasesConfig:
    if not path.is_file():
        raise FileNotFoundError(f"gate cases config not found: {path}")

    payload = tomllib.loads(path.read_text(encoding="utf-8"))
    root = _expect_dict(payload, "root")
    markdown_payload = _expect_dict(root.get("markdown"), "[markdown]")
    triplet_payload = _expect_dict(root.get("triplet"), "[triplet]")

    return GateCasesConfig(
        markdown=_load_markdown_cases(markdown_payload),
        triplet=_load_triplet_cases(triplet_payload),
    )
