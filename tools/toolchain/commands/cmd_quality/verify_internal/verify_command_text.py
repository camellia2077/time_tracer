from __future__ import annotations


def build_verify_command_text(
    *,
    app_name: str,
    profile_name: str | None,
    build_dir_name: str | None,
    concise: bool,
    tidy: bool,
    kill_build_procs: bool,
    cmake_args: list[str] | None,
) -> str:
    cmd = ["python", "tools/run.py", "verify", "--app", app_name]
    if profile_name:
        cmd.extend(["--profile", profile_name])
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
