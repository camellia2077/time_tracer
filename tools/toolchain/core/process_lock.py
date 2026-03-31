from __future__ import annotations

import json
import os
import time
from contextlib import contextmanager
from pathlib import Path

if os.name == "nt":
    import msvcrt
else:
    import fcntl


class ProcessLockBusyError(RuntimeError):
    def __init__(
        self,
        *,
        lock_path: Path,
        label: str,
        owner_description: str = "",
    ) -> None:
        self.lock_path = lock_path
        self.label = label
        self.owner_description = owner_description.strip()
        super().__init__(self.render_user_message())

    def render_user_message(self) -> str:
        lines = [
            f"--- {self.label} is already running in this workspace.",
            "--- Android Gradle commands must be serialized. Wait for the active run to finish and retry.",
            f"--- Lock file: {self.lock_path.as_posix()}",
        ]
        if self.owner_description:
            lines.append(f"--- Active owner: {self.owner_description}")
        return "\n".join(lines)


def _ensure_lockfile_exists(handle) -> None:
    handle.seek(0, os.SEEK_END)
    if handle.tell() <= 0:
        handle.write(" ")
        handle.flush()
    handle.seek(0)


def _try_lock_handle(handle) -> None:
    _ensure_lockfile_exists(handle)
    if os.name == "nt":
        msvcrt.locking(handle.fileno(), msvcrt.LK_NBLCK, 1)
    else:
        fcntl.flock(handle.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)


def _unlock_handle(handle) -> None:
    handle.seek(0)
    if os.name == "nt":
        msvcrt.locking(handle.fileno(), msvcrt.LK_UNLCK, 1)
    else:
        fcntl.flock(handle.fileno(), fcntl.LOCK_UN)


def _read_owner_description(lock_path: Path) -> str:
    try:
        payload = lock_path.read_text(encoding="utf-8").strip()
    except OSError:
        return ""

    if not payload:
        return ""

    try:
        data = json.loads(payload)
    except json.JSONDecodeError:
        return payload

    parts: list[str] = []
    pid = data.get("pid")
    if pid:
        parts.append(f"pid={pid}")
    started_at = data.get("started_at")
    if started_at:
        parts.append(f"started_at={started_at}")
    command = str(data.get("command") or "").strip()
    if command:
        parts.append(f"command={command}")
    cwd = str(data.get("cwd") or "").strip()
    if cwd:
        parts.append(f"cwd={cwd}")
    return ", ".join(parts)


@contextmanager
def hold_process_lock(
    *,
    lock_path: Path,
    label: str,
    metadata: dict[str, object] | None = None,
):
    lock_path.parent.mkdir(parents=True, exist_ok=True)
    with open(lock_path, "a+", encoding="utf-8") as handle:
        try:
            _try_lock_handle(handle)
        except OSError as exc:
            raise ProcessLockBusyError(
                lock_path=lock_path,
                label=label,
                owner_description=_read_owner_description(lock_path),
            ) from exc

        payload = dict(metadata or {})
        payload.setdefault("pid", os.getpid())
        payload.setdefault("started_at", time.strftime("%Y-%m-%d %H:%M:%S"))
        handle.seek(0)
        handle.truncate()
        handle.write(json.dumps(payload, ensure_ascii=True))
        handle.flush()
        try:
            yield
        finally:
            handle.seek(0)
            handle.truncate()
            handle.write(" ")
            handle.flush()
            _unlock_handle(handle)
