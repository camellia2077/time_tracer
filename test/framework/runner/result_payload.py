from pathlib import Path
from typing import Any


def resolve_result_json_path(args, suite_root: Path, suite_output_root: Path) -> Path:
    canonical_result_json = (suite_output_root / "result.json").resolve()
    if not args.result_json:
        return canonical_result_json

    requested_result_json = Path(args.result_json)
    if not requested_result_json.is_absolute():
        requested_result_json = (suite_root / requested_result_json).resolve()

    if requested_result_json != canonical_result_json:
        print(
            "Warning: output contract enforces result JSON path at "
            f"{canonical_result_json}; ignoring --result-json={requested_result_json}"
        )

    return canonical_result_json


def resolve_result_cases_json_path(suite_output_root: Path) -> Path:
    return (suite_output_root / "result_cases.json").resolve()


def extract_case_records(result_payload: Any) -> list[dict[str, Any]]:
    if not isinstance(result_payload, dict):
        return []

    raw_records = result_payload.pop("_case_records", None)
    if not isinstance(raw_records, list):
        return []

    records: list[dict[str, Any]] = []
    for item in raw_records:
        if isinstance(item, dict):
            records.append(item)
    return records


def build_result_cases_payload(
    result_payload: Any,
    case_records: list[dict[str, Any]],
) -> dict[str, Any]:
    failed_cases: list[dict[str, Any]] = [
        item for item in case_records if item.get("status") == "FAIL"
    ]

    total_cases = len(case_records)
    total_failed = len(failed_cases)
    if isinstance(result_payload, dict):
        total_cases = int(result_payload.get("total_tests", total_cases) or 0)
        total_failed = int(result_payload.get("total_failed", total_failed) or 0)

    return {
        "success": (total_failed == 0),
        "total_cases": total_cases,
        "total_failed": total_failed,
        "cases": failed_cases,
    }
