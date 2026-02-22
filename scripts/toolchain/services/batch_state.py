import json
from datetime import UTC, datetime
from pathlib import Path

from ..core.context import Context


def state_path(ctx: Context, app_name: str) -> Path:
    return ctx.get_app_dir(app_name) / "build_tidy" / "batch_state.json"


def load_state(path: Path, app_name: str) -> dict:
    state = _default_state(app_name)
    if not path.exists():
        return state
    try:
        payload = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return state
    if not isinstance(payload, dict):
        return state

    merged = dict(state)
    merged.update(payload)

    cleaned_task_ids = merged.get("cleaned_task_ids")
    if not isinstance(cleaned_task_ids, list):
        merged["cleaned_task_ids"] = []
    else:
        normalized: list[str] = []
        for task_id in cleaned_task_ids:
            text = str(task_id).strip()
            if not text:
                continue
            if text not in normalized:
                normalized.append(text)
        merged["cleaned_task_ids"] = normalized
    merged["app"] = app_name
    return merged


def update_state(
    ctx: Context,
    app_name: str,
    batch_id: str | None = None,
    cleaned_task_ids: list[str] | None = None,
    last_verify_success: bool | None = None,
    last_refresh_ok: bool | None = None,
    extra_fields: dict | None = None,
) -> Path:
    path = state_path(ctx, app_name)
    state = load_state(path, app_name)
    if batch_id:
        state["batch_id"] = batch_id
    if cleaned_task_ids is not None:
        existing = state.get("cleaned_task_ids", [])
        for task_id in cleaned_task_ids:
            text = str(task_id).strip()
            if not text:
                continue
            if text not in existing:
                existing.append(text)
        state["cleaned_task_ids"] = existing
    if last_verify_success is not None:
        state["last_verify_success"] = bool(last_verify_success)
    if last_refresh_ok is not None:
        state["last_refresh_ok"] = bool(last_refresh_ok)
    if extra_fields:
        state.update(extra_fields)
    state["updated_at"] = _utc_now_iso()
    _write_state(path, state)
    return path


def _default_state(app_name: str) -> dict:
    return {
        "version": 1,
        "app": app_name,
        "batch_id": None,
        "cleaned_task_ids": [],
        "last_verify_success": None,
        "last_refresh_ok": None,
        "updated_at": None,
    }


def _write_state(path: Path, state: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        json.dumps(state, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )


def _utc_now_iso() -> str:
    return datetime.now(UTC).isoformat()
