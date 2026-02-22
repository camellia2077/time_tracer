from pathlib import Path

from ...services.clangd_lsp import ClangdClient
from . import apply_processor


class RenameApplyMixin:
    def _process_apply_candidate(
        self,
        index: int,
        candidate: dict,
        app_dir: Path,
        clangd_client: ClangdClient,
        effective_dry_run: bool,
    ) -> dict:
        return apply_processor.process_apply_candidate(
            command=self,
            index=index,
            candidate=candidate,
            app_dir=app_dir,
            clangd_client=clangd_client,
            effective_dry_run=effective_dry_run,
        )

    def apply(
        self,
        app_name: str,
        limit: int = 0,
        dry_run: bool = False,
        strict: bool = False,
    ) -> int:
        paths = self._paths(app_name)
        candidates = self._load_candidates(paths["candidates_path"])
        if not candidates:
            print("--- No rename candidates found.")
            print("--- Run `python scripts/run.py rename-plan --app <app>` first.")
            return 1

        selected_candidates = candidates[:limit] if limit > 0 else candidates
        effective_dry_run = dry_run or self.ctx.config.rename.dry_run_default
        print(
            f"--- Rename apply start: {len(selected_candidates)} candidate(s), dry_run={effective_dry_run}"
        )

        ret = self._ensure_tidy_build_ready(app_name)
        if ret != 0:
            print("--- Failed to prepare tidy build for clangd.")
            return ret

        clangd_client = ClangdClient(
            clangd_path=self.ctx.config.rename.clangd_path,
            compile_commands_dir=paths["build_tidy_dir"],
            root_dir=paths["app_dir"],
            background_index=self.ctx.config.rename.clangd_background_index,
        )

        results = []
        status_counts = {"applied": 0, "skipped": 0, "failed": 0}

        try:
            if not self._start_clangd(clangd_client):
                return 1

            self._wait_for_clangd_warmup()

            for index, candidate in enumerate(selected_candidates, 1):
                result = self._process_apply_candidate(
                    index=index,
                    candidate=candidate,
                    app_dir=paths["app_dir"],
                    clangd_client=clangd_client,
                    effective_dry_run=effective_dry_run,
                )
                status_counts[result["status"]] += 1
                results.append(result)
        finally:
            clangd_client.stop()

        self._write_apply_report(
            json_path=paths["apply_report_json_path"],
            markdown_path=paths["apply_report_md_path"],
            app_name=app_name,
            dry_run=effective_dry_run,
            status_counts=status_counts,
            results=results,
        )

        print("--- Rename apply complete.")
        print(
            f"--- Applied: {status_counts['applied']}, "
            f"Skipped: {status_counts['skipped']}, Failed: {status_counts['failed']}"
        )
        print(f"--- Report JSON: {paths['apply_report_json_path']}")
        print(f"--- Report Markdown: {paths['apply_report_md_path']}")

        if strict and status_counts["failed"] > 0:
            return 1
        return 0
