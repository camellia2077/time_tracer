from __future__ import annotations

import os
import shutil
from pathlib import Path


def resolve_cargo_executable(env: dict[str, str]) -> str:
    resolved = shutil.which("cargo", path=env.get("PATH"))
    if resolved:
        return resolved

    for candidate in _collect_cargo_fallback_candidates(env):
        if candidate.exists():
            print(f"--- build: cargo executable resolved via fallback: {candidate}")
            return str(candidate)

    return "cargo"


def _collect_cargo_fallback_candidates(env: dict[str, str]) -> list[Path]:
    if os.name == "nt":
        return _collect_windows_cargo_candidates(env)
    return _collect_posix_cargo_candidates(env)


def _collect_windows_cargo_candidates(env: dict[str, str]) -> list[Path]:
    candidates: list[Path] = []
    user_profile = _normalize_windows_like_path(
        env.get("USERPROFILE") or os.environ.get("USERPROFILE") or ""
    )
    if user_profile:
        candidates.append(Path(user_profile) / ".cargo" / "bin" / "cargo.exe")

    home_drive = (env.get("HOMEDRIVE") or os.environ.get("HOMEDRIVE") or "").strip()
    home_path = (env.get("HOMEPATH") or os.environ.get("HOMEPATH") or "").strip()
    if home_drive and home_path:
        candidates.append(Path(f"{home_drive}{home_path}") / ".cargo" / "bin" / "cargo.exe")

    home_value = _normalize_windows_like_path(env.get("HOME") or os.environ.get("HOME") or "")
    if home_value:
        candidates.append(Path(home_value) / ".cargo" / "bin" / "cargo.exe")
    return candidates


def _collect_posix_cargo_candidates(env: dict[str, str]) -> list[Path]:
    candidates: list[Path] = []
    home_value = (env.get("HOME") or os.environ.get("HOME") or "").strip()
    if home_value:
        candidates.append(Path(home_value) / ".cargo" / "bin" / "cargo")
    return candidates


def _normalize_windows_like_path(raw_path: str) -> str:
    text = (raw_path or "").strip()
    if not text:
        return ""
    lowered = text.lower()
    if lowered.startswith("/mnt/") and len(text) > 6 and text[5].isalpha() and text[6] == "/":
        drive = text[5].upper()
        tail = text[7:]
        return f"{drive}:/{tail}"
    if lowered.startswith("/") and len(text) > 3 and text[1].isalpha() and text[2] == "/":
        drive = text[1].upper()
        tail = text[3:]
        return f"{drive}:/{tail}"
    return text
