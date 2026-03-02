import json
from typing import Any


def extract_json_payload(text: str) -> tuple[Any | None, str | None]:
    raw = (text or "").strip()
    if not raw:
        return None, "empty output"

    try:
        return json.loads(raw), None
    except Exception:
        pass

    first_obj = raw.find("{")
    last_obj = raw.rfind("}")
    if first_obj != -1 and last_obj > first_obj:
        try:
            return json.loads(raw[first_obj : last_obj + 1]), None
        except Exception:
            pass

    first_arr = raw.find("[")
    last_arr = raw.rfind("]")
    if first_arr != -1 and last_arr > first_arr:
        try:
            return json.loads(raw[first_arr : last_arr + 1]), None
        except Exception:
            pass

    return None, "invalid JSON payload"


def tokenize_json_path(path: str) -> list[str | int]:
    tokens: list[str | int] = []
    normalized = (path or "").strip()
    if not normalized:
        raise ValueError("JSON path is empty.")

    for segment in normalized.split("."):
        cursor = segment.strip()
        if not cursor:
            raise ValueError(f"Invalid empty JSON path segment in `{path}`.")

        while cursor:
            if cursor.startswith("["):
                end = cursor.find("]")
                if end <= 1:
                    raise ValueError(f"Invalid list index segment in `{path}`.")
                index_text = cursor[1:end].strip()
                if not index_text.isdigit():
                    raise ValueError(f"List index must be unsigned integer in `{path}`.")
                tokens.append(int(index_text))
                cursor = cursor[end + 1 :].strip()
                continue

            bracket = cursor.find("[")
            if bracket == -1:
                tokens.append(cursor)
                cursor = ""
            else:
                key = cursor[:bracket].strip()
                if key:
                    tokens.append(key)
                cursor = cursor[bracket:].strip()

    return tokens


def resolve_json_path(payload: Any, path: str) -> tuple[bool, Any | None, str | None]:
    try:
        tokens = tokenize_json_path(path)
    except ValueError as error:
        return False, None, str(error)

    current = payload
    for token in tokens:
        if isinstance(token, int):
            if not isinstance(current, list):
                return False, None, f"Path `{path}` expects list for index `{token}`."
            if token < 0 or token >= len(current):
                return False, None, f"Path `{path}` index out of range: {token}."
            current = current[token]
            continue

        if not isinstance(current, dict):
            return False, None, f"Path `{path}` expects object for key `{token}`."
        if token not in current:
            return False, None, f"Path `{path}` missing key `{token}`."
        current = current[token]

    return True, current, None


def parse_expected_json_value(raw: str) -> Any:
    text = (raw or "").strip()
    if text == "":
        return ""
    try:
        return json.loads(text)
    except Exception:
        return raw
