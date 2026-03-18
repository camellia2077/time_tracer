from __future__ import annotations

import hashlib
import json
import shlex
import subprocess
import time
from collections import Counter
from pathlib import Path

from ...core.context import Context
from ..cmd_build import BuildCommand
from ..tidy import invoker as tidy_invoker
from .sarif import extract_primary_location
from .split import DEFAULT_ANALYZE_BATCH_SIZE, split_sarif_report
from . import workspace as analyze_workspace

_WRAPPER_NAMES = {"ccache", "ccache.exe", "sccache", "sccache.exe"}
_STRIP_ARG_NAMES = {
    "-c",
    "-MD",
    "-MMD",
    "-MP",
}
_STRIP_ARG_NAMES_WITH_VALUE = {
    "-o",
    "-MF",
    "-MT",
    "-MQ",
    "-MJ",
    "--serialize-diagnostics",
    "-dependency-file",
}


def _format_progress_file(repo_root: Path, file_path: Path) -> str:
    try:
        return file_path.resolve().relative_to(repo_root.resolve()).as_posix()
    except ValueError:
        return file_path.resolve().as_posix()


def _parse_compile_command(entry: dict) -> list[str]:
    arguments = entry.get("arguments")
    if isinstance(arguments, list) and arguments:
        return [str(arg) for arg in arguments]
    command_text = entry.get("command")
    if isinstance(command_text, str) and command_text.strip():
        return shlex.split(command_text, posix=False)
    raise ValueError("compile_commands entry is missing both `arguments` and `command`")


def _resolve_entry_file(entry: dict) -> Path:
    file_text = str(entry.get("file") or "").strip()
    if not file_text:
        raise ValueError("compile_commands entry is missing `file`")
    file_path = Path(file_text)
    if file_path.is_absolute():
        return file_path.resolve()
    directory_text = str(entry.get("directory") or "").strip()
    base_dir = Path(directory_text) if directory_text else Path.cwd()
    return (base_dir / file_path).resolve()


def _is_under_any_root(file_path: Path, roots: list[Path]) -> bool:
    if not roots:
        return True
    file_text = str(file_path).replace("\\", "/").lower()
    for root in roots:
        root_text = str(root).replace("\\", "/").lower().rstrip("/")
        if file_text == root_text or file_text.startswith(root_text + "/"):
            return True
    return False


def _is_excluded_analyze_file(file_path: Path, repo_root: Path) -> bool:
    normalized = str(file_path.resolve()).replace("\\", "/").lower()
    if "/_deps/" in normalized:
        return True

    try:
        file_path.resolve().relative_to((repo_root / "out").resolve())
        return True
    except ValueError:
        return False


def _strip_build_only_flags(args: list[str]) -> list[str]:
    stripped: list[str] = []
    index = 0
    while index < len(args):
        arg = args[index]
        if arg in _STRIP_ARG_NAMES:
            index += 1
            continue
        if arg in _STRIP_ARG_NAMES_WITH_VALUE:
            index += 2
            continue
        if arg.startswith("/Fo"):
            index += 1
            continue
        stripped.append(arg)
        index += 1
    return stripped


def build_analyzer_command(entry: dict, output_path: Path) -> list[str]:
    raw_args = _parse_compile_command(entry)
    if not raw_args:
        raise ValueError("empty compile command")

    prefix: list[str] = []
    index = 0
    while index < len(raw_args) and Path(raw_args[index]).name.lower() in _WRAPPER_NAMES:
        prefix.append(raw_args[index])
        index += 1
    if index >= len(raw_args):
        raise ValueError("compile command does not contain a compiler executable")

    compiler = raw_args[index]
    remaining = _strip_build_only_flags(raw_args[index + 1 :])
    return [
        *prefix,
        compiler,
        "--analyze",
        "--analyzer-output",
        "sarif",
        "-o",
        str(output_path),
        *remaining,
    ]

def merge_sarif_reports(report_paths: list[Path]) -> dict:
    merged = {
        "$schema": (
            "https://docs.oasis-open.org/sarif/sarif/v2.1.0/cos02/"
            "schemas/sarif-schema-2.1.0.json"
        ),
        "runs": [],
    }
    for report_path in report_paths:
        payload = json.loads(report_path.read_text(encoding="utf-8"))
        runs = payload.get("runs")
        if isinstance(runs, list):
            merged["runs"].extend(runs)
    return merged


def build_summary(
    *,
    workspace_name: str,
    source_scope: str | None,
    build_dir: Path,
    raw_report_path: Path,
    log_path: Path,
    matched_units: int,
    analyzed_units: int,
    failed_units: list[dict[str, object]],
    merged_sarif: dict,
) -> dict:
    rule_counts: Counter[str] = Counter()
    file_counts: Counter[str] = Counter()
    result_count = 0
    for run in merged_sarif.get("runs", []):
        if not isinstance(run, dict):
            continue
        for result in run.get("results", []):
            if not isinstance(result, dict):
                continue
            result_count += 1
            rule_id = str(result.get("ruleId") or "unknown").strip() or "unknown"
            rule_counts[rule_id] += 1
            primary_location = extract_primary_location(run, result)
            source_path = str(primary_location.get("file") or "").strip()
            if source_path:
                file_counts[source_path] += 1
    return {
        "version": 1,
        "workspace": workspace_name,
        "source_scope": source_scope,
        "build_dir": str(build_dir),
        "raw_report": str(raw_report_path),
        "log": str(log_path),
        "totals": {
            "matched_units": matched_units,
            "analyzed_units": analyzed_units,
            "failed_units": len(failed_units),
            "results": result_count,
            "files_with_findings": len(file_counts),
        },
        "top_rules": [
            {"rule_id": rule_id, "count": count}
            for rule_id, count in rule_counts.most_common(10)
        ],
        "top_files": [
            {"file": file_path, "count": count}
            for file_path, count in file_counts.most_common(10)
        ],
        "failures": failed_units[:20],
    }


class AnalyzeCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    def execute(
        self,
        app_name: str,
        *,
        jobs: int | None = None,
        source_scope: str | None = None,
        build_dir_name: str | None = None,
        profile_name: str | None = None,
    ) -> int:
        workspace = analyze_workspace.resolve_workspace(
            self.ctx,
            build_dir_name=build_dir_name,
            source_scope=source_scope,
        )
        analyze_layout = self.ctx.get_analyze_layout(app_name, workspace.build_dir_name)
        build_dir = self.ctx.get_build_dir(app_name, workspace.build_dir_name)
        analyze_layout.root.mkdir(parents=True, exist_ok=True)
        analyze_layout.reports_dir.mkdir(parents=True, exist_ok=True)
        analyze_layout.unit_reports_dir.mkdir(parents=True, exist_ok=True)

        overall_start = time.perf_counter()
        configure_seconds = 0.0
        did_auto_configure = False
        cache_path = build_dir / "CMakeCache.txt"
        if not cache_path.exists():
            print(f"--- Analyze build directory {build_dir} not configured. Running auto-configure...")
            configure_start = time.perf_counter()
            builder = BuildCommand(self.ctx)
            ret = builder.configure(
                app_name=app_name,
                tidy=False,
                source_scope=workspace.source_scope,
                build_dir_name=workspace.build_dir_name,
                profile_name=profile_name,
            )
            configure_seconds = time.perf_counter() - configure_start
            did_auto_configure = True
            if ret != 0:
                print("--- Auto-configure failed. Aborting analyze.")
                return ret

        filtered_jobs = jobs if jobs is not None else 0
        if workspace.prebuild_targets:
            prebuild_log_path = build_dir / "module_prereq_build.log"
            prebuild_cmd = tidy_invoker.build_module_prereq_command(
                build_dir,
                workspace.prebuild_targets,
                filtered_jobs if filtered_jobs > 0 else None,
            )
            print("--- Analyze module prebuild: " + ", ".join(workspace.prebuild_targets))
            prebuild_ret, _ = tidy_invoker.run_tidy_build(self.ctx, prebuild_cmd, prebuild_log_path)
            if prebuild_ret != 0:
                print(f"--- Analyze module prebuild failed with code {prebuild_ret}.")
                return prebuild_ret

        compile_commands_path = build_dir / "compile_commands.json"
        if not compile_commands_path.exists():
            print(f"--- Missing compile_commands.json: {compile_commands_path}")
            return 1

        payload = json.loads(compile_commands_path.read_text(encoding="utf-8"))
        if not isinstance(payload, list):
            print(f"--- Invalid compile_commands payload: {compile_commands_path}")
            return 1

        matched_entries: list[tuple[dict, Path]] = []
        for entry in payload:
            if not isinstance(entry, dict):
                continue
            try:
                file_path = _resolve_entry_file(entry)
            except ValueError:
                continue
            if _is_excluded_analyze_file(file_path, self.ctx.repo_root):
                continue
            if _is_under_any_root(file_path, workspace.source_roots):
                matched_entries.append((entry, file_path))

        analyze_layout.output_log_path.write_text("", encoding="utf-8")

        env = self.ctx.setup_env()
        report_paths: list[Path] = []
        failed_units: list[dict[str, object]] = []
        analyzed_units = 0
        total_units = len(matched_entries)

        for unit_index, (entry, file_path) in enumerate(matched_entries, start=1):
            digest = hashlib.sha1(str(file_path).encode("utf-8")).hexdigest()[:10]
            unit_report_path = analyze_layout.unit_reports_dir / f"unit_{unit_index:05d}_{digest}.sarif"
            display_file = _format_progress_file(self.ctx.repo_root, file_path)
            try:
                analyzer_cmd = build_analyzer_command(entry, unit_report_path)
            except ValueError as error:
                print(
                    f"Analyze [{unit_index}/{total_units}] {display_file}",
                    flush=True,
                )
                print(
                    f"Analyze [{unit_index}/{total_units}] skipped: {error}",
                    flush=True,
                )
                failed_units.append(
                    {
                        "file": str(file_path),
                        "exit_code": 1,
                        "error": str(error),
                    }
                )
                continue

            print(
                f"Analyze [{unit_index}/{total_units}] {display_file}",
                flush=True,
            )

            with analyze_layout.output_log_path.open("a", encoding="utf-8") as log_handle:
                log_handle.write(f"=== [{unit_index}/{len(matched_entries)}] {file_path}\n")
                log_handle.write("--- Running: " + " ".join(str(arg) for arg in analyzer_cmd) + "\n")

            completed = subprocess.run(
                analyzer_cmd,
                cwd=Path(str(entry.get("directory") or build_dir)),
                env=env,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                encoding="utf-8",
                errors="replace",
                check=False,
            )
            with analyze_layout.output_log_path.open("a", encoding="utf-8") as log_handle:
                if completed.stdout:
                    log_handle.write(completed.stdout)
                    if not completed.stdout.endswith("\n"):
                        log_handle.write("\n")
                log_handle.write(f"--- Exit code: {completed.returncode}\n\n")

            if completed.returncode != 0:
                print(
                    f"Analyze [{unit_index}/{total_units}] failed (exit {completed.returncode})",
                    flush=True,
                )
                failed_units.append(
                    {
                        "file": str(file_path),
                        "exit_code": completed.returncode,
                    }
                )
                continue

            analyzed_units += 1
            print(
                f"Analyze [{unit_index}/{total_units}] done",
                flush=True,
            )
            if unit_report_path.exists():
                report_paths.append(unit_report_path)

        merged_sarif = merge_sarif_reports(report_paths)
        analyze_layout.raw_report_path.write_text(
            json.dumps(merged_sarif, indent=2, ensure_ascii=False),
            encoding="utf-8",
        )
        summary = build_summary(
            workspace_name=workspace.build_dir_name,
            source_scope=workspace.source_scope,
            build_dir=build_dir,
            raw_report_path=analyze_layout.raw_report_path,
            log_path=analyze_layout.output_log_path,
            matched_units=len(matched_entries),
            analyzed_units=analyzed_units,
            failed_units=failed_units,
            merged_sarif=merged_sarif,
        )
        analyze_layout.summary_json_path.write_text(
            json.dumps(summary, indent=2, ensure_ascii=False),
            encoding="utf-8",
        )

        auto_split_stats: dict[str, object] | None = None
        if not failed_units:
            auto_split_stats = split_sarif_report(
                raw_report_path=analyze_layout.raw_report_path,
                issues_dir=analyze_layout.issues_dir,
                summary_path=analyze_layout.summary_json_path,
                workspace_name=workspace.build_dir_name,
                source_scope=workspace.source_scope,
                batch_size=DEFAULT_ANALYZE_BATCH_SIZE,
            )

        total_seconds = time.perf_counter() - overall_start
        print(f"--- Analyze workspace: {analyze_layout.root}")
        print(f"--- Analyze units matched: {len(matched_entries)}")
        print(f"--- Analyze units completed: {analyzed_units}")
        print(f"--- Analyze units failed: {len(failed_units)}")
        print(f"--- Analyze results: {summary['totals']['results']}")
        print(f"--- Analyze raw SARIF: {analyze_layout.raw_report_path}")
        print(f"--- Analyze summary: {analyze_layout.summary_json_path}")
        if auto_split_stats is not None:
            print(
                "--- Analyze split summary: "
                f"issues={auto_split_stats['issues']}, "
                f"batches={auto_split_stats['batches']}, "
                f"batch_size={auto_split_stats['batch_size']}, "
                f"issues_dir={analyze_layout.issues_dir}"
            )
        print(
            "--- Analyze timing: "
            f"configure={configure_seconds:.2f}s "
            f"total={total_seconds:.2f}s "
            f"auto_configure={'yes' if did_auto_configure else 'no'}"
        )
        return 0 if not failed_units else 1

    def split_only(
        self,
        app_name: str,
        *,
        source_scope: str | None = None,
        build_dir_name: str | None = None,
        batch_size: int | None = None,
    ) -> int:
        workspace = analyze_workspace.resolve_workspace(
            self.ctx,
            build_dir_name=build_dir_name,
            source_scope=source_scope,
        )
        analyze_layout = self.ctx.get_analyze_layout(app_name, workspace.build_dir_name)
        raw_report_path = analyze_layout.raw_report_path
        if not raw_report_path.exists():
            print(f"--- analyze-split: raw SARIF not found: {raw_report_path}")
            print("--- analyze-split: run `analyze` first to generate reports/run.sarif.")
            return 1

        effective_batch_size = (
            DEFAULT_ANALYZE_BATCH_SIZE if batch_size is None else batch_size
        )
        try:
            split_stats = split_sarif_report(
                raw_report_path=raw_report_path,
                issues_dir=analyze_layout.issues_dir,
                summary_path=analyze_layout.summary_json_path,
                workspace_name=workspace.build_dir_name,
                source_scope=workspace.source_scope,
                batch_size=effective_batch_size,
            )
        except ValueError as error:
            print(f"--- analyze-split: invalid split settings: {error}")
            return 1

        print(
            "--- analyze-split summary: "
            f"issues={split_stats['issues']}, "
            f"batches={split_stats['batches']}, "
            f"batch_size={split_stats['batch_size']}, "
            f"issues_dir={analyze_layout.issues_dir}"
        )
        return 0
