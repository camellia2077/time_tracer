import json
from datetime import UTC, datetime
from pathlib import Path

from ..commands.tidy.workspace import DEFAULT_TIDY_BUILD_DIR_NAME


def state_path(ctx, app_name: str, tidy_build_dir_name: str | None = None) -> Path:
    resolved = (tidy_build_dir_name or "").strip() or DEFAULT_TIDY_BUILD_DIR_NAME
    return ctx.get_tidy_layout(app_name, resolved).tidy_state_path


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
    state.update(payload)
    cleaned = state.get("cleaned_task_ids")
    if not isinstance(cleaned, list):
        state["cleaned_task_ids"] = []
    else:
        state["cleaned_task_ids"] = list(dict.fromkeys(str(item).strip() for item in cleaned if str(item).strip()))
    state["app"] = app_name
    return state


def update_state(
    ctx,
    app_name: str,
    tidy_build_dir_name: str | None = None,
    cluster_id: str | None = None,
    cleaned_task_ids: list[str] | None = None,
    last_verify_success: bool | None = None,
    last_refresh_ok: bool | None = None,
    extra_fields: dict | None = None,
) -> Path:
    path = state_path(ctx, app_name, tidy_build_dir_name)
    state = load_state(path, app_name)
    if cluster_id:
        state["cluster_id"] = cluster_id
    if cleaned_task_ids is not None:
        existing = state.setdefault("cleaned_task_ids", [])
        state["cleaned_task_ids"] = list(dict.fromkeys([*existing, *[str(item).strip() for item in cleaned_task_ids if str(item).strip()]]))
    if last_verify_success is not None:
        state["last_verify_success"] = bool(last_verify_success)
    if last_refresh_ok is not None:
        state["last_refresh_ok"] = bool(last_refresh_ok)
    if extra_fields:
        state.update(extra_fields)
    state["updated_at"] = datetime.now(UTC).isoformat()
    _write_state(path, state)
    return path


def _default_state(app_name: str) -> dict:
    return {
        "version": 2,
        "app": app_name,
        "cluster_id": None,
        "scan_id": None,
        "cleaned_task_ids": [],
        "last_verify_success": None,
        "last_refresh_ok": None,
        "queue_requires_reresolve": False,
        "next_queue_head": None,
        "replacement_queue_head": None,
        "queue_transition_summary": None,
        "next_action": None,
        "final_gate": None,
        "updated_at": None,
    }


def _write_state(path: Path, state: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(state, ensure_ascii=False, indent=2), encoding="utf-8")
