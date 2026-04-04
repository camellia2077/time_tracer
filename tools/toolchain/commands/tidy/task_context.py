from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

from ...core.context import Context
from .task_fingerprint import compute_source_fingerprint, fingerprints_match
from .task_log import TASK_FILE_PATTERN, load_task_record
from .task_model import SourceFingerprint, TaskRecord
from .task_queue import read_queue_generation
from .workspace import DEFAULT_TIDY_BUILD_DIR_NAME, normalize_source_scope


@dataclass(frozen=True, slots=True)
class ResolvedTaskContext:
    app_name: str
    tidy_build_dir_name: str
    source_scope: str | None
    tasks_dir: Path
    task_json_path: Path
    parsed_task: TaskRecord
    current_queue_generation: int | None
    current_source_fingerprint: SourceFingerprint | None
    source_fingerprint_matches: bool


def resolve_task_context(
    ctx: Context,
    *,
    task_log_path: str | Path | None,
) -> ResolvedTaskContext:
    task_json_path = resolve_explicit_task_json_path(
        ctx.repo_root,
        task_log_path=task_log_path,
    )
    app_name, tidy_build_dir_name, tasks_dir = _parse_task_artifact_layout(task_json_path)
    parsed_task = load_task_record(task_json_path)
    current_queue_generation = read_queue_generation(tasks_dir)
    if (
        parsed_task.queue_generation is not None
        and current_queue_generation is not None
        and parsed_task.queue_generation != current_queue_generation
    ):
        raise ValueError(
            "task queue generation is stale; re-resolve the current queue from tasks/ "
            "before running task-local commands."
        )

    recorded_workspace = str(parsed_task.workspace or "").strip()
    if recorded_workspace and recorded_workspace != tidy_build_dir_name:
        raise ValueError(
            "task workspace does not match task path: "
            f"{recorded_workspace} != {tidy_build_dir_name}"
        )

    source_scope = normalize_source_scope(parsed_task.source_scope)
    if source_scope is None:
        source_scope = infer_source_scope_from_build_dir(ctx, tidy_build_dir_name)

    current_source_fingerprint = compute_source_fingerprint(parsed_task.source_file)

    return ResolvedTaskContext(
        app_name=app_name,
        tidy_build_dir_name=tidy_build_dir_name,
        source_scope=source_scope,
        tasks_dir=tasks_dir,
        task_json_path=task_json_path,
        parsed_task=parsed_task,
        current_queue_generation=current_queue_generation,
        current_source_fingerprint=current_source_fingerprint,
        source_fingerprint_matches=fingerprints_match(
            parsed_task.source_fingerprint,
            current_source_fingerprint,
        ),
    )


def resolve_explicit_task_json_path(
    repo_root: Path,
    *,
    task_log_path: str | Path | None,
) -> Path:
    raw_path = task_log_path
    if isinstance(raw_path, Path):
        candidate = raw_path.expanduser()
    else:
        text = str(raw_path or "").strip()
        if not text:
            raise ValueError("`--task-log` is required.")
        candidate = Path(text).expanduser()

    if not candidate.is_absolute():
        candidate = (repo_root / candidate).resolve()
    else:
        candidate = candidate.resolve()

    if not TASK_FILE_PATTERN.match(candidate.name):
        raise ValueError(
            "`--task-log` must point to a task_*.json/.toon/.log artifact."
        )

    if candidate.suffix.lower() != ".json":
        json_path = candidate.with_suffix(".json")
        if not json_path.exists():
            raise FileNotFoundError(f"canonical task json not found: {json_path}")
        candidate = json_path

    if not candidate.exists():
        raise FileNotFoundError(f"task artifact not found: {candidate}")
    return candidate


def infer_source_scope_from_build_dir(ctx: Context, build_dir_name: str) -> str | None:
    matches: list[str] = []
    for scope_name, scope_cfg in ctx.config.tidy.source_scopes.items():
        scoped_build_dir = str(getattr(scope_cfg, "tidy_build_dir", "") or "").strip()
        if scoped_build_dir == build_dir_name:
            matches.append(scope_name)
    if not matches:
        if build_dir_name == DEFAULT_TIDY_BUILD_DIR_NAME:
            return None
        raise ValueError(
            "cannot infer clang-tidy source scope from task path build dir "
            f"`{build_dir_name}`"
        )
    if len(matches) > 1:
        joined = ", ".join(sorted(matches))
        raise ValueError(
            "ambiguous clang-tidy source scope for build dir "
            f"`{build_dir_name}`: {joined}"
        )
    return matches[0]


def _parse_task_artifact_layout(task_json_path: Path) -> tuple[str, str, Path]:
    batch_dir = task_json_path.parent
    tasks_dir = batch_dir.parent
    if tasks_dir.name != "tasks":
        raise ValueError(
            "task path must live under out/tidy/<app>/<workspace>/tasks/<batch>/task_xxx.json"
        )

    tidy_build_dir = tasks_dir.parent
    app_dir = tidy_build_dir.parent
    tidy_dir = app_dir.parent
    out_dir = tidy_dir.parent
    if tidy_dir.name != "tidy" or out_dir.name != "out":
        raise ValueError(
            "task path must live under out/tidy/<app>/<workspace>/tasks/<batch>/task_xxx.json"
        )

    return app_dir.name, tidy_build_dir.name, tasks_dir
