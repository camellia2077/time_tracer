from __future__ import annotations

from dataclasses import dataclass, field

from ...core.context import Context
from .classifier import (
    is_cmake_related,
    is_documentation_file,
    is_script_file,
    is_source_file,
    is_test_related,
)
from .git_query import collect_changed_files

_RUN_TESTS_MODES = {"auto", "always", "never"}
_SCRIPT_CHANGE_MODES = {"auto", "build", "skip"}


@dataclass
class ChangePolicyDecision:
    app_name: str
    build_dir_name: str
    changed_files: list[str] = field(default_factory=list)
    need_configure: bool = False
    need_build: bool = False
    need_test: bool = False
    cmake_args: list[str] = field(default_factory=list)
    reasons: list[str] = field(default_factory=list)


def decide_post_change_policy(
    ctx: Context,
    app_name: str | None,
    build_dir_name: str,
    run_tests_mode: str,
    script_changes_mode: str,
) -> ChangePolicyDecision:
    normalized_run_tests_mode, run_tests_warning = _normalize_mode(
        raw_value=run_tests_mode,
        allowed_modes=_RUN_TESTS_MODES,
        fallback="auto",
        mode_name="run-tests",
    )
    normalized_script_changes_mode, script_changes_warning = _normalize_mode(
        raw_value=script_changes_mode,
        allowed_modes=_SCRIPT_CHANGE_MODES,
        fallback="build",
        mode_name="script-changes",
    )
    changed_files = collect_changed_files(ctx.repo_root)
    resolved_app_name, app_reason = _resolve_app_name(
        ctx=ctx,
        explicit_app_name=app_name,
        changed_files=changed_files,
    )

    decision = ChangePolicyDecision(
        app_name=resolved_app_name,
        build_dir_name=build_dir_name,
        changed_files=changed_files,
    )
    decision.reasons.append(app_reason)
    if run_tests_warning:
        decision.reasons.append(run_tests_warning)
    if script_changes_warning:
        decision.reasons.append(script_changes_warning)
    decision.reasons.append(f"script-changes mode: {normalized_script_changes_mode}.")

    if not changed_files:
        decision.reasons.append("No local file changes detected.")
        _apply_test_mode_override(decision, normalized_run_tests_mode)
        if decision.need_test:
            decision.need_build = True
            decision.reasons.append("run-tests=always requested; enabling build before tests.")
        return decision

    non_doc_files = [path for path in changed_files if not is_documentation_file(path)]
    docs_only = len(non_doc_files) == 0
    has_script_change = any(is_script_file(path) for path in non_doc_files)
    only_script_changes = bool(non_doc_files) and all(
        is_script_file(path) for path in non_doc_files
    )
    has_cmake_change = any(is_cmake_related(path) for path in changed_files)
    has_test_change = any(is_test_related(path) for path in changed_files)
    has_source_change = any(is_source_file(path) for path in changed_files)

    if docs_only:
        decision.reasons.append("Only documentation/meta files changed. Build skipped.")
    else:
        if has_cmake_change or has_source_change or has_test_change:
            decision.need_build = True
            decision.reasons.append(
                "Build-impacting changes detected (cmake/source/test); build is required."
            )
        elif only_script_changes:
            if normalized_script_changes_mode == "build":
                decision.need_build = True
                decision.reasons.append(
                    "Script-only changes with script-changes=build; build is required."
                )
            elif normalized_script_changes_mode == "skip":
                decision.reasons.append(
                    "Script-only changes with script-changes=skip; build skipped."
                )
            else:
                decision.reasons.append(
                    "Script-only changes with script-changes=auto; build skipped."
                )
        else:
            decision.need_build = True
            if has_script_change:
                decision.reasons.append(
                    "Mixed script + non-doc changes detected; build is required."
                )
            else:
                decision.reasons.append("Non-documentation changes detected; build is required.")

    if has_cmake_change:
        decision.need_configure = True
        decision.reasons.append("CMake-related changes detected; configure is required.")
    elif decision.need_build:
        decision.reasons.append(
            "No CMake-related changes detected; rely on auto-configure when cache is missing."
        )

    if has_test_change:
        decision.need_test = True
        decision.reasons.append("Test-related changes detected; tests are required.")
    elif has_source_change:
        decision.reasons.append("Source changes detected; tests remain optional in auto mode.")

    _apply_test_mode_override(decision, normalized_run_tests_mode)
    if decision.need_test and not decision.need_build:
        decision.need_build = True
        decision.reasons.append("Tests requested without build; enabling build.")

    return decision


def _apply_test_mode_override(decision: ChangePolicyDecision, run_tests_mode: str) -> None:
    if run_tests_mode == "always":
        decision.need_test = True
        decision.reasons.append("run-tests=always override applied.")
        return
    if run_tests_mode == "never":
        decision.need_test = False
        decision.reasons.append("run-tests=never override applied.")


def _resolve_app_name(
    ctx: Context,
    explicit_app_name: str | None,
    changed_files: list[str],
) -> tuple[str, str]:
    if explicit_app_name:
        return explicit_app_name, f"Using explicit app from CLI: {explicit_app_name}."

    configured_default_app = (ctx.config.post_change.default_app or "").strip()
    configured_apps = list(ctx.config.apps.keys())
    if not configured_apps:
        fallback = configured_default_app or "tracer_core"
        return fallback, f"No configured apps found; fallback to `{fallback}`."

    default_app_is_valid = (
        configured_default_app in configured_apps if configured_default_app else False
    )
    fallback_app = (
        configured_default_app
        if default_app_is_valid
        else ("tracer_core" if "tracer_core" in configured_apps else configured_apps[0])
    )
    invalid_default_note = ""
    if configured_default_app and not default_app_is_valid:
        invalid_default_note = (
            f" Configured default app `{configured_default_app}` is invalid; "
            f"fallback to `{fallback_app}`."
        )

    app_prefixes: dict[str, str] = {}
    for configured_app_name in configured_apps:
        app_dir = ctx.get_app_dir(configured_app_name)
        try:
            rel = app_dir.relative_to(ctx.repo_root).as_posix().rstrip("/")
        except ValueError:
            continue
        if rel:
            app_prefixes[configured_app_name] = rel

    touched_apps: list[str] = []
    for configured_app_name, rel_prefix in app_prefixes.items():
        prefix = f"{rel_prefix}/"
        if any(path == rel_prefix or path.startswith(prefix) for path in changed_files):
            touched_apps.append(configured_app_name)

    if len(touched_apps) == 1:
        app = touched_apps[0]
        return app, f"Auto-selected app from changed paths: {app}."

    if len(touched_apps) > 1:
        touched_apps_text = ", ".join(sorted(touched_apps))
        return (
            fallback_app,
            f"Multiple apps touched ({touched_apps_text}); fallback to {fallback_app}.{invalid_default_note}",
        )

    if default_app_is_valid:
        return (
            fallback_app,
            f"No app-specific path hit; fallback to configured default app `{fallback_app}`.",
        )
    return (
        fallback_app,
        f"No app-specific path hit; fallback to {fallback_app}.{invalid_default_note}",
    )


def _normalize_mode(
    raw_value: str,
    allowed_modes: set[str],
    fallback: str,
    mode_name: str,
) -> tuple[str, str | None]:
    normalized = (raw_value or "").strip().lower()
    if normalized in allowed_modes:
        return normalized, None
    warning = f"Unknown {mode_name} mode `{raw_value}`; fallback to `{fallback}`."
    return fallback, warning
