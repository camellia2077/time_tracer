import time
from pathlib import Path

from ...services.clangd_lsp import ClangdClient
from ..cmd_build import BuildCommand
from . import common_paths


def ensure_tidy_build_ready(ctx, app_name: str) -> int:
    build_tidy_dir = ctx.get_app_dir(app_name) / "build_tidy"
    compile_commands_path = build_tidy_dir / "compile_commands.json"
    if compile_commands_path.exists():
        return 0

    print(f"--- Missing {compile_commands_path}. Running tidy configure...")
    builder = BuildCommand(ctx)
    return builder.configure(app_name, tidy=True)


def extract_candidate_fields(
    candidate: dict, app_dir: Path
) -> tuple[Path, str, str, str, int, int]:
    file_path = common_paths.resolve_file_path(candidate.get("file", ""), app_dir)
    old_name = candidate.get("old_name", "")
    new_name = candidate.get("new_name", "")
    symbol_kind = candidate.get("symbol_kind", "")
    line = int(candidate.get("line", 0))
    col = int(candidate.get("col", 0))
    return file_path, old_name, new_name, symbol_kind, line, col


def build_apply_result(
    index: int,
    status: str,
    reason: str,
    file_path: Path,
    line: int,
    col: int,
    old_name: str,
    new_name: str,
    edit_count: int = 0,
    changed_files: list[str] | None = None,
) -> dict:
    return {
        "id": index,
        "status": status,
        "reason": reason,
        "file": str(file_path),
        "line": line,
        "col": col,
        "old_name": old_name,
        "new_name": new_name,
        "edit_count": edit_count,
        "changed_files": changed_files or [],
    }


def start_clangd(clangd_client: ClangdClient) -> bool:
    try:
        clangd_client.start()
        return True
    except Exception as exc:
        print(f"--- Failed to start clangd: {exc}")
        return False


def wait_for_clangd_warmup(ctx) -> None:
    warmup_seconds = max(0.0, float(ctx.config.rename.clangd_warmup_seconds))
    if warmup_seconds > 0:
        print(f"--- Waiting {warmup_seconds:.1f}s for clangd index warm-up...")
        time.sleep(warmup_seconds)
