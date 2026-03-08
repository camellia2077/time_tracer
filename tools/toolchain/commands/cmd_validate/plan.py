from __future__ import annotations

import re
import tomllib
from dataclasses import dataclass, field
from pathlib import Path

_VALID_KINDS = {"configure", "build", "verify"}
_VALID_VERIFY_SCOPES = {"task", "unit", "artifact", "batch"}


@dataclass(frozen=True)
class TrackSpec:
    name: str
    kind: str
    app: str
    profile: str | None = None
    build_dir: str | None = None
    cmake_args: list[str] = field(default_factory=list)
    verify_scope: str = "batch"
    concise: bool = True
    kill_build_procs: bool = False


@dataclass(frozen=True)
class ValidationPlan:
    plan_path: Path
    run_name: str
    continue_on_failure: bool
    tracks: list[TrackSpec]


def normalize_run_name(raw_name: str) -> str:
    candidate = re.sub(r"[^A-Za-z0-9._-]+", "_", (raw_name or "").strip()).strip("._-")
    return candidate or "validate_run"


def _coerce_bool(value: object, *, default: bool) -> bool:
    if isinstance(value, bool):
        return value
    return default


def _coerce_optional_string(value: object) -> str | None:
    if value is None:
        return None
    text = str(value).strip()
    return text or None


def _coerce_string_list(value: object) -> list[str]:
    if value is None:
        return []
    if not isinstance(value, list):
        raise ValueError("expected a list of strings")
    result: list[str] = []
    for item in value:
        text = str(item).strip()
        if text:
            result.append(text)
    return result


def _normalize_kind(raw_kind: object) -> str:
    kind = str(raw_kind or "verify").strip().lower()
    if kind not in _VALID_KINDS:
        raise ValueError(f"unsupported track kind `{kind}`")
    return kind


def _normalize_verify_scope(raw_scope: object) -> str:
    scope = str(raw_scope or "batch").strip().lower()
    if scope not in _VALID_VERIFY_SCOPES:
        raise ValueError(f"unsupported verify scope `{scope}`")
    return scope


def load_validation_plan(plan_path: Path, run_name_override: str | None = None) -> ValidationPlan:
    resolved_plan_path = plan_path.resolve()
    if not resolved_plan_path.exists():
        raise FileNotFoundError(f"validation plan not found: {resolved_plan_path}")

    with open(resolved_plan_path, "rb") as handle:
        payload = tomllib.load(handle)

    if not isinstance(payload, dict):
        raise ValueError("validation plan root must be a TOML table")

    run_table = payload.get("run", {})
    defaults_table = payload.get("defaults", {})
    tracks_table = payload.get("tracks", [])

    if not isinstance(run_table, dict):
        raise ValueError("[run] must be a TOML table")
    if not isinstance(defaults_table, dict):
        raise ValueError("[defaults] must be a TOML table")
    if not isinstance(tracks_table, list) or not tracks_table:
        raise ValueError("validation plan must define at least one [[tracks]] entry")

    declared_run_name = _coerce_optional_string(run_table.get("name")) or resolved_plan_path.stem
    run_name = normalize_run_name(run_name_override or declared_run_name)
    continue_on_failure = _coerce_bool(run_table.get("continue_on_failure"), default=True)

    default_kind = _normalize_kind(defaults_table.get("kind", "verify"))
    default_app = _coerce_optional_string(defaults_table.get("app"))
    default_profile = _coerce_optional_string(defaults_table.get("profile"))
    default_build_dir = _coerce_optional_string(defaults_table.get("build_dir"))
    default_cmake_args = _coerce_string_list(defaults_table.get("cmake_args"))
    default_verify_scope = _normalize_verify_scope(defaults_table.get("verify_scope", "batch"))
    default_concise = _coerce_bool(defaults_table.get("concise"), default=True)
    default_kill_build_procs = _coerce_bool(defaults_table.get("kill_build_procs"), default=False)

    tracks: list[TrackSpec] = []
    for index, item in enumerate(tracks_table, start=1):
        if not isinstance(item, dict):
            raise ValueError(f"[[tracks]] entry #{index} must be a TOML table")

        name = _coerce_optional_string(item.get("name")) or f"track_{index:02d}"
        kind = _normalize_kind(item.get("kind", default_kind))
        app = _coerce_optional_string(item.get("app")) or default_app
        if not app:
            raise ValueError(f"track `{name}` is missing `app` and no default app is set")

        tracks.append(
            TrackSpec(
                name=name,
                kind=kind,
                app=app,
                profile=_coerce_optional_string(item.get("profile", default_profile)),
                build_dir=_coerce_optional_string(item.get("build_dir", default_build_dir)),
                cmake_args=_coerce_string_list(item.get("cmake_args", default_cmake_args)),
                verify_scope=_normalize_verify_scope(
                    item.get("verify_scope", default_verify_scope)
                ),
                concise=_coerce_bool(item.get("concise"), default=default_concise),
                kill_build_procs=_coerce_bool(
                    item.get("kill_build_procs"),
                    default=default_kill_build_procs,
                ),
            )
        )

    return ValidationPlan(
        plan_path=resolved_plan_path,
        run_name=run_name,
        continue_on_failure=continue_on_failure,
        tracks=tracks,
    )
