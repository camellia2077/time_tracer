from pathlib import Path

from ...services.clangd_lsp import ClangdClient
from . import common_paths, common_runtime, common_symbols


class RenameCommonMixin:
    def _paths(self, app_name: str) -> dict[str, Path]:
        return common_paths.paths(self.ctx, app_name)

    def _load_candidates(self, candidates_path: Path) -> list[dict]:
        return common_paths.load_candidates(candidates_path)

    def _is_within_app_scope(self, file_path: Path, app_dir: Path) -> bool:
        return common_paths.is_within_app_scope(file_path, app_dir)

    def _resolve_file_path(self, file_path: str, app_dir: Path) -> Path:
        return common_paths.resolve_file_path(file_path, app_dir)

    def _resolve_position(
        self,
        file_path: Path,
        line: int,
        col: int,
        old_name: str,
    ) -> tuple[int, int]:
        return common_symbols.resolve_position(file_path, line, col, old_name)

    def _is_header_file(self, file_path: Path) -> bool:
        return common_symbols.is_header_file(file_path)

    def _is_risky_symbol_kind(self, symbol_kind: str) -> bool:
        return common_symbols.is_risky_symbol_kind(symbol_kind)

    def _should_skip_partial_header_rename(
        self,
        symbol_kind: str,
        source_file: Path,
        edit_count: int,
        changed_files: list[str],
    ) -> bool:
        return common_symbols.should_skip_partial_header_rename(
            skip_header_single_edit=self.ctx.config.rename.skip_header_single_edit,
            symbol_kind=symbol_kind,
            source_file=source_file,
            edit_count=edit_count,
            changed_files=changed_files,
        )

    def _count_symbol_occurrences(self, text: str, symbol_name: str) -> int:
        return common_symbols.count_symbol_occurrences(text, symbol_name)

    def _count_symbol_in_sibling_sources(
        self,
        header_path: Path,
        old_name: str,
        new_name: str,
    ) -> tuple[int, int]:
        return common_symbols.count_symbol_in_sibling_sources(header_path, old_name, new_name)

    def _ensure_tidy_build_ready(self, app_name: str) -> int:
        return common_runtime.ensure_tidy_build_ready(self.ctx, app_name)

    def _extract_candidate_fields(
        self,
        candidate: dict,
        app_dir: Path,
    ) -> tuple[Path, str, str, str, int, int]:
        return common_runtime.extract_candidate_fields(candidate, app_dir)

    def _build_apply_result(
        self,
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
        return common_runtime.build_apply_result(
            index=index,
            status=status,
            reason=reason,
            file_path=file_path,
            line=line,
            col=col,
            old_name=old_name,
            new_name=new_name,
            edit_count=edit_count,
            changed_files=changed_files,
        )

    def _start_clangd(self, clangd_client: ClangdClient) -> bool:
        return common_runtime.start_clangd(clangd_client)

    def _wait_for_clangd_warmup(self) -> None:
        common_runtime.wait_for_clangd_warmup(self.ctx)
