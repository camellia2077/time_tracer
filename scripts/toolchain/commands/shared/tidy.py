import json
import re
from pathlib import Path

from ...core.context import Context
from ...services.suite_registry import resolve_suite_name

BATCH_NAME_PATTERN = re.compile(r"^batch_(\d+)$")
BATCH_ID_PATTERN = re.compile(r"^\d+$")


def normalize_batch_name(
    batch_id: str | None,
    *,
    allow_none: bool,
) -> str | None:
    if batch_id is None:
        if allow_none:
            return None
        raise ValueError("empty --batch-id")

    raw = batch_id.strip()
    if not raw:
        if allow_none:
            return None
        raise ValueError("empty --batch-id")

    match = BATCH_NAME_PATTERN.match(raw)
    if match:
        return f"batch_{int(match.group(1)):03d}"
    if BATCH_ID_PATTERN.match(raw):
        return f"batch_{int(raw):03d}"
    raise ValueError("invalid --batch-id. Use 1/001 or batch_001 style identifiers.")


def normalize_required_batch_name(batch_id: str) -> str:
    normalized = normalize_batch_name(batch_id, allow_none=False)
    if normalized is None:
        raise ValueError("empty --batch-id")
    return normalized


def read_json_dict(
    path: Path,
    *,
    encoding: str = "utf-8",
) -> dict | None:
    if not path.exists():
        return None
    try:
        payload = json.loads(path.read_text(encoding=encoding))
    except (OSError, json.JSONDecodeError):
        return None
    if not isinstance(payload, dict):
        return None
    return payload


def write_json_dict(
    path: Path,
    payload: dict,
    *,
    encoding: str = "utf-8",
) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        json.dumps(payload, ensure_ascii=False, indent=2),
        encoding=encoding,
    )


def latest_verify_succeeded(ctx: Context, app_name: str) -> tuple[bool, str]:
    suite_name = resolve_suite_name(app_name) or app_name
    result_path = ctx.repo_root / "test" / "output" / suite_name / "result.json"
    payload = read_json_dict(result_path)
    if payload is None:
        return False, f"missing or invalid {result_path}"

    success = bool(payload.get("success", False))
    if success:
        return True, "success=true"
    exit_code = payload.get("exit_code")
    return False, f"success=false exit_code={exit_code}"
