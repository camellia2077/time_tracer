from __future__ import annotations

import os
from pathlib import Path

_SUITE_BY_APP = {
    # `tracer_windows_cli` is the integrated core + Windows CLI suite.
    "tracer_core": "tracer_windows_cli",
    "tracer_windows_cli": "tracer_windows_cli",
    "tracer_android": "tracer_android",
    "log_generator": "log_generator",
}

_SUITE_BUILD_APP = {
    "tracer_windows_cli": "tracer_windows_cli",
    "tracer_android": "tracer_android",
    "log_generator": "log_generator",
}


def _resolve_windows_shell_bin_dir() -> str | None:
    comspec = (os.environ.get("ComSpec") or "").strip()
    if comspec:
        comspec_path = Path(comspec)
        if comspec_path.exists():
            return str(comspec_path.parent)

    system_root = (os.environ.get("SystemRoot") or "").strip()
    if system_root:
        system32 = Path(system_root) / "System32"
        if system32.exists():
            return str(system32)

    fallback = Path("C:/Windows/System32")
    if fallback.exists():
        return str(fallback)
    return None


_SUITE_RUNNER_BIN_DIR = {
    "tracer_android": _resolve_windows_shell_bin_dir,
}


def resolve_suite_name(app_name: str) -> str | None:
    return _SUITE_BY_APP.get(app_name)


def resolve_suite_build_app(app_name: str) -> str | None:
    suite_name = resolve_suite_name(app_name)
    if not suite_name:
        return None
    return _SUITE_BUILD_APP.get(suite_name)


def resolve_suite_bin_dir(app_name: str) -> str | None:
    suite_name = resolve_suite_name(app_name)
    if not suite_name:
        return None

    resolver = _SUITE_RUNNER_BIN_DIR.get(suite_name)
    if resolver is None:
        return None
    if callable(resolver):
        return resolver()
    return resolver


def needs_suite_build(app_name: str) -> bool:
    suite_build_app = resolve_suite_build_app(app_name)
    return suite_build_app is not None and suite_build_app != app_name
