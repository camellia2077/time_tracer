from __future__ import annotations

from collections.abc import Iterable


def build_verify_command_text(
    *,
    app_name: str,
    profile_name: str | Iterable[str] | None,
    build_dir_name: str | None,
    concise: bool,
    tidy: bool,
    kill_build_procs: bool,
    cmake_args: list[str] | None,
) -> str:
    cmd = ["python", "tools/run.py", "verify", "--app", app_name]
    if profile_name:
        if isinstance(profile_name, str):
            cmd.extend(["--profile", profile_name])
        else:
            for selected_profile in profile_name:
                cmd.extend(["--profile", str(selected_profile)])
    if build_dir_name:
        cmd.extend(["--build-dir", build_dir_name])
    if concise:
        cmd.append("--concise")
    if tidy:
        cmd.append("--tidy")
    if kill_build_procs:
        cmd.append("--kill-build-procs")
    for arg in cmake_args or []:
        cmd.append(f"--cmake-args={arg}")
    return " ".join(cmd)
