import re
from pathlib import Path
from typing import Any

from ..conf.definitions import AppExitCode
from .json_payload import (
    extract_json_payload,
    parse_expected_json_value,
    resolve_json_path,
)


def evaluate_command_expectations(
    *,
    return_code: int,
    stdout_text: str,
    stderr_text: str,
    expect_exit: int | None,
    expectations: dict[str, Any],
) -> tuple[str, list[str]]:
    status = "PASS"
    messages: list[str] = []

    def fail(message: str) -> None:
        nonlocal status
        status = "FAIL"
        messages.append(message)

    if expect_exit is not None and return_code != expect_exit:
        err_msg = AppExitCode.to_string(return_code)
        fail(f"Expected exit {expect_exit}, got {return_code} ({err_msg}).")

    for needle in expectations.get("expect_stdout_contains", []) or []:
        if needle not in stdout_text:
            fail(f"Missing stdout text: {needle}")

    for pattern in expectations.get("expect_stdout_regex", []) or []:
        try:
            matched = re.search(pattern, stdout_text, flags=re.MULTILINE) is not None
        except re.error as error:
            fail(f"Invalid stdout regex `{pattern}`: {error}")
            continue
        if not matched:
            fail(f"stdout regex not matched: {pattern}")

    stdout_any_of = [str(item) for item in (expectations.get("expect_stdout_any_of", []) or [])]
    if stdout_any_of and not any(needle in stdout_text for needle in stdout_any_of):
        fail("stdout missing all alternatives: " + " | ".join(stdout_any_of))

    for needle in expectations.get("expect_stderr_contains", []) or []:
        if needle not in stderr_text:
            fail(f"Missing stderr text: {needle}")

    expect_json_fields = expectations.get("expect_json_fields", []) or []
    expect_error_code = expectations.get("expect_error_code")
    expect_error_category = expectations.get("expect_error_category")
    expect_hints_contains = [
        str(item) for item in (expectations.get("expect_hints_contains", []) or [])
    ]

    parsed_stdout_json, stdout_json_error = extract_json_payload(stdout_text)
    parsed_stderr_json, stderr_json_error = extract_json_payload(stderr_text)
    preferred_json_payload = (
        parsed_stdout_json
        if isinstance(parsed_stdout_json, (dict, list))
        else parsed_stderr_json
        if isinstance(parsed_stderr_json, (dict, list))
        else None
    )

    if expect_json_fields:
        if preferred_json_payload is None:
            fail(
                "expect_json_fields requested, but no JSON payload found "
                f"(stdout: {stdout_json_error}; stderr: {stderr_json_error})."
            )
        else:
            for spec in expect_json_fields:
                path = str(spec)
                expected_value: Any | None = None
                has_expected = False

                if "::" in path:
                    path, expected_raw = path.split("::", 1)
                    expected_value = parse_expected_json_value(expected_raw)
                    has_expected = True

                found, actual_value, path_error = resolve_json_path(
                    preferred_json_payload,
                    path,
                )
                if not found:
                    fail(path_error or f"Missing JSON path: {path}")
                    continue

                if has_expected:
                    if isinstance(expected_value, str):
                        if str(actual_value) != expected_value:
                            fail(
                                f"JSON field mismatch `{path}`: expected `{expected_value}`, got `{actual_value}`"
                            )
                    elif actual_value != expected_value:
                        fail(
                            f"JSON field mismatch `{path}`: expected `{expected_value}`, got `{actual_value}`"
                        )

    combined_text = f"{stdout_text}\n{stderr_text}"
    if expect_error_code:
        actual_error_code = ""
        if isinstance(preferred_json_payload, dict):
            value = preferred_json_payload.get("error_code")
            if isinstance(value, str):
                actual_error_code = value.strip()
        if not actual_error_code:
            bracket_match = re.search(r"\[([a-zA-Z0-9_.-]+)\]", combined_text)
            if bracket_match:
                actual_error_code = bracket_match.group(1).strip()

        if actual_error_code:
            if actual_error_code != expect_error_code:
                fail(
                    f"Error code mismatch: expected `{expect_error_code}`, got `{actual_error_code}`"
                )
        elif str(expect_error_code) not in combined_text:
            fail(f"Missing error code: {expect_error_code}")

    if expect_error_category:
        actual_error_category = ""
        if isinstance(preferred_json_payload, dict):
            value = preferred_json_payload.get("error_category")
            if isinstance(value, str):
                actual_error_category = value.strip()
        if not actual_error_category:
            category_match = re.search(r"category=([a-zA-Z0-9_.-]+)", combined_text)
            if category_match:
                actual_error_category = category_match.group(1).strip()

        if actual_error_category:
            if actual_error_category != expect_error_category:
                fail(
                    "Error category mismatch: "
                    f"expected `{expect_error_category}`, got `{actual_error_category}`"
                )
        elif str(expect_error_category) not in combined_text:
            fail(f"Missing error category: {expect_error_category}")

    if expect_hints_contains:
        actual_hints: list[str] = []
        if isinstance(preferred_json_payload, dict):
            hints_value = preferred_json_payload.get("hints")
            if isinstance(hints_value, list):
                actual_hints = [str(item).strip() for item in hints_value if str(item).strip()]
        if not actual_hints:
            hints_match = re.search(r"hints=([^)]+)", combined_text)
            if hints_match:
                actual_hints = [
                    segment.strip()
                    for segment in hints_match.group(1).split("|")
                    if segment.strip()
                ]

        if not actual_hints:
            fail("Missing error hints in output.")
        else:
            hints_text = " | ".join(actual_hints)
            for hint in expect_hints_contains:
                if hint not in hints_text:
                    fail(f"Missing error hint: {hint}")

    for path_str in expectations.get("expect_files", []) or []:
        if not Path(path_str).exists():
            fail(f"Missing file: {path_str}")

    for spec in expectations.get("expect_file_contains", []) or []:
        if "::" not in spec:
            fail(f"Invalid expect_file_contains spec (missing '::'): {spec}")
            continue

        path_str, needle = spec.split("::", 1)
        path_obj = Path(path_str)
        if not path_obj.exists():
            fail(f"Missing file for content check: {path_str}")
            continue

        try:
            content = path_obj.read_text(encoding="utf-8", errors="replace")
        except Exception as error:
            fail(f"Failed to read file for content check: {path_str} ({error})")
            continue

        if needle not in content:
            fail(f"Missing file content in {path_str}: {needle}")

    return status, messages
