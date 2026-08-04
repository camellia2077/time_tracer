import argparse
import json
from datetime import datetime
from pathlib import Path

from .config import (
    LANG_CHOICES,
    LanguageConfig,
    load_language_config,
)
from .classification import (
    resolve_single_language_category,
)
from .reporter import LocConsoleReporter
from .service import UNDER_SENTINEL, LocScanService, ScanArgumentResolver
from .profile_scan import ProfileScanRunner


class LocCliApplication:
    DIR_OVER_SENTINEL = -1
    _LOG_DIR_GITIGNORE_CONTENT = "# Automatically created by loc_scanner.\n*\n"

    def run(self) -> int:
        args = self.parse_args()
        workspace_root = self._resolve_workspace_root(args.workspace_root)
        config_path = self._resolve_config_path(args.config, workspace_root=workspace_root)
        log_path = self._resolve_log_path(
            args.log_file,
            args.lang,
            profile=args.profile,
            workspace_root=workspace_root,
        )
        log_path.parent.mkdir(parents=True, exist_ok=True)

        exit_code, payload = self._run_scan(
            args,
            workspace_root=workspace_root,
            config_path=config_path,
        )
        self._write_json_log(log_path, payload)

        print(f"[LOG] Scan log: {log_path}")
        return exit_code

    def _run_scan(
        self,
        args: argparse.Namespace,
        *,
        workspace_root: Path,
        config_path: Path,
    ) -> tuple[int, dict]:
        payload = self._build_base_payload(args=args, workspace_root=workspace_root)

        if args.dir_max_depth is not None and args.dir_over_files is None:
            return self._error(
                "--dir-max-depth can only be used with --dir-over-files.",
                payload,
            )

        if args.profile is not None:
            return self._run_profile_scan(
                args,
                payload,
                workspace_root=workspace_root,
                config_path=config_path,
            )

        if args.compare_baseline:
            return self._error(
                "--compare-baseline can only be used with --profile.",
                payload,
            )

        try:
            config = load_language_config(config_path=config_path, lang=args.lang)
        except (FileNotFoundError, ValueError, OSError) as error:
            return self._error(f"Configuration loading failed: {error}", payload)

        resolver = ScanArgumentResolver()
        scan_service = LocScanService(config)
        reporter = LocConsoleReporter(config)
        paths = resolver.resolve_paths(
            args.paths,
            config.default_paths,
            config.path_mode,
            workspace_root=workspace_root,
        )

        if args.dir_over_files is not None:
            return self._run_dir_file_scan(args, payload, paths, config, scan_service, reporter)

        mode, threshold = resolver.resolve_mode_and_threshold(args, config)
        if threshold <= 0:
            return self._error("Threshold must be a positive integer.", payload)
        return self._run_line_scan(
            payload=payload,
            paths=paths,
            mode=mode,
            threshold=threshold,
            scan_service=scan_service,
            reporter=reporter,
        )

    def _run_profile_scan(
        self,
        args: argparse.Namespace,
        payload: dict,
        *,
        workspace_root: Path,
        config_path: Path,
    ) -> tuple[int, dict]:
        return ProfileScanRunner(
            workspace_root=workspace_root,
            config_path=config_path,
        ).run(args, payload)

    def _run_line_scan(
        self,
        *,
        payload: dict,
        paths: list[Path],
        mode: str,
        threshold: int,
        scan_service: LocScanService,
        reporter: LocConsoleReporter,
    ) -> tuple[int, dict]:
        reporter.print_line_scan_header(mode, threshold)
        path_results: list[dict] = []
        total_matched_files = 0

        for path in paths:
            if not path.exists():
                reporter.print_missing_path(path)
                path_results.append(
                    {
                        "path": str(path),
                        "matched_files": [],
                        "category_counts": {
                            "production": 0,
                            "tests": 0,
                            "other": 0,
                        },
                    }
                )
                continue
            matched = scan_service.analyze_path(path, mode, threshold)
            matched_files = [
                {
                    "path": file_path,
                    "lines": lines,
                    "category": resolve_single_language_category(
                        file_path=Path(file_path),
                        scan_root=path,
                        test_directory_names=scan_service.config.test_directory_names,
                    ),
                }
                for file_path, lines in matched
            ]
            category_counts = {
                category: sum(
                    file_result["category"] == category
                    for file_result in matched_files
                )
                for category in ("production", "tests", "other")
            }
            reporter.print_line_path_result(path, mode, threshold, matched_files)
            total_matched_files += len(matched_files)
            path_results.append(
                {
                    "path": str(path),
                    "matched_files": matched_files,
                    "category_counts": category_counts,
                }
            )

        payload["status"] = "ok"
        payload["scan"] = {"mode": mode, "threshold": threshold}
        payload["results"] = path_results
        payload["summary"] = {
            "matched_files": total_matched_files,
            "category_counts": {
                category: sum(
                    path_result["category_counts"][category]
                    for path_result in path_results
                )
                for category in ("production", "tests", "other")
            },
        }
        return 0, payload

    def _run_dir_file_scan(
        self,
        args: argparse.Namespace,
        payload: dict,
        paths: list[Path],
        config: LanguageConfig,
        scan_service: LocScanService,
        reporter: LocConsoleReporter,
    ) -> tuple[int, dict]:
        if args.dir_over_files == self.DIR_OVER_SENTINEL:
            threshold = config.default_dir_over_files
        else:
            threshold = int(args.dir_over_files)

        if threshold <= 0:
            return self._error("--dir-over-files threshold must be a positive integer.", payload)
        if args.dir_max_depth is not None and args.dir_max_depth < 0:
            return self._error("--dir-max-depth must be an integer >= 0.", payload)

        reporter.print_dir_scan_header(threshold, args.dir_max_depth)
        path_results: list[dict] = []
        total_matched_dirs = 0

        for path in paths:
            if not path.exists():
                reporter.print_missing_path(path)
                path_results.append({"path": str(path), "matched_dirs": []})
                continue
            matched = scan_service.analyze_directory_file_counts(
                path,
                threshold=threshold,
                max_depth=args.dir_max_depth,
            )
            reporter.print_dir_path_result(path, threshold, matched)
            matched_dirs = [
                {"path": dir_path, "files": file_count} for dir_path, file_count in matched
            ]
            total_matched_dirs += len(matched_dirs)
            path_results.append({"path": str(path), "matched_dirs": matched_dirs})

        payload["status"] = "ok"
        payload["scan"] = {"mode": "dir_over_files", "threshold": threshold}
        if args.dir_max_depth is not None:
            payload["scan"]["max_depth"] = args.dir_max_depth
        payload["results"] = path_results
        payload["summary"] = {"matched_dirs": total_matched_dirs}
        return 0, payload

    @staticmethod
    def parse_args() -> argparse.Namespace:
        default_config = str(LocCliApplication._default_config_path())
        parser = argparse.ArgumentParser(
            description="Unified line-count scanner for C++, Kotlin, Python, and Rust."
        )
        selector = parser.add_mutually_exclusive_group(required=True)
        selector.add_argument(
            "--lang",
            choices=LANG_CHOICES,
            help="Scan one language: cpp | kt | py | rs.",
        )
        selector.add_argument(
            "--profile",
            metavar="NAME",
            help="Scan multiple components and languages through a TOML profile.",
        )
        parser.add_argument(
            "paths",
            nargs="*",
            help=(
                "Directories to scan (multiple relative or absolute paths are supported). "
                "Uses TOML default_paths when omitted; ignored when path_mode=toml_only."
            ),
        )
        parser.add_argument(
            "--workspace-root",
            default=".",
            help="Workspace root used to resolve relative paths for paths, config, and log-file.",
        )
        parser.add_argument(
            "--config",
            default=default_config,
            help=f"TOML configuration path. Default: {default_config}",
        )
        parser.add_argument(
            "--log-file",
            default=None,
            help=(
                "Scan log output path (relative or absolute). "
                "Defaults to scan_<lang>.json for language scans and scan_profile_<name>.json for profiles."
            ),
        )

        group = parser.add_mutually_exclusive_group()
        group.add_argument("--over", type=int, metavar="N", help="Scan large files (over mode).")
        group.add_argument(
            "--under",
            type=int,
            nargs="?",
            const=UNDER_SENTINEL,
            metavar="N",
            help="Scan small files (under mode). Uses TOML default_under_threshold when N is omitted.",
        )
        group.add_argument(
            "--dir-over-files",
            type=int,
            nargs="?",
            const=LocCliApplication.DIR_OVER_SENTINEL,
            metavar="N",
            help=(
                "Scan directories containing more than N code files. "
                "Uses the language default_dir_over_files when N is omitted."
            ),
        )
        parser.add_argument("-t", "--threshold", type=int, help="Legacy alias equivalent to --over N.")
        parser.add_argument(
            "--dir-max-depth",
            type=int,
            default=None,
            help="Maximum directory scan depth relative to the input root; 0 scans only the root. Used with --dir-over-files.",
        )
        parser.add_argument(
            "--compare-baseline",
            default=None,
            metavar="PATH",
            help=(
                "Compare this profile scan with a previously generated profile JSON report "
                "and report added, removed, and changed hotspots."
            ),
        )
        parser.add_argument(
            "--save-baseline",
            default=None,
            metavar="PATH",
            help="Save this profile JSON report to an independent baseline path for --compare-baseline.",
        )
        return parser.parse_args()

    @staticmethod
    def _default_config_path() -> Path:
        return (Path(__file__).resolve().parents[2] / "config" / "scan_lines.toml").resolve()

    @staticmethod
    def _resolve_workspace_root(workspace_root: str) -> Path:
        return Path(workspace_root).resolve()

    @staticmethod
    def _resolve_config_path(config_path: str, *, workspace_root: Path) -> Path:
        path = Path(config_path)
        if not path.is_absolute():
            path = workspace_root / path
        return path.resolve()

    @staticmethod
    def _resolve_log_path(
        log_file: str | None,
        lang: str | None,
        *,
        profile: str | None = None,
        workspace_root: Path,
    ) -> Path:
        if log_file:
            path = Path(log_file)
            if not path.is_absolute():
                path = (workspace_root / path).resolve()
            else:
                path = path.resolve()
            return path
        scan_name = f"profile_{profile}" if profile else lang
        return (workspace_root / "temp" / "loc_scanner" / "logs" / f"scan_{scan_name}.json").resolve()

    @staticmethod
    def _build_base_payload(*, args: argparse.Namespace, workspace_root: Path) -> dict:
        payload = {
            "generated_at": datetime.now().astimezone().isoformat(timespec="seconds"),
            "status": "unknown",
            "workspace_root": str(workspace_root),
        }
        if getattr(args, "lang", None) is not None:
            payload["lang"] = args.lang
        if getattr(args, "profile", None) is not None:
            payload["profile"] = args.profile
        return payload

    @staticmethod
    def _write_json_log(path: Path, payload: dict) -> None:
        path.write_text(json.dumps(payload, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
        LocCliApplication._ensure_log_dir_gitignore(path.parent)

    @staticmethod
    def _ensure_log_dir_gitignore(log_dir: Path) -> None:
        gitignore_path = log_dir / ".gitignore"
        required_lines = ("# Automatically created by loc_scanner.", "*")
        if not gitignore_path.exists():
            gitignore_path.write_text(
                LocCliApplication._LOG_DIR_GITIGNORE_CONTENT,
                encoding="utf-8",
            )
            return

        existing_lines = gitignore_path.read_text(encoding="utf-8").splitlines()
        missing = [line for line in required_lines if line not in existing_lines]
        if not missing:
            return

        output_lines = existing_lines[:]
        if output_lines and output_lines[-1].strip():
            output_lines.append("")
        output_lines.extend(missing)
        gitignore_path.write_text("\n".join(output_lines) + "\n", encoding="utf-8")

    @staticmethod
    def _error(message: str, payload: dict) -> tuple[int, dict]:
        print(f"[ERROR] {message}")
        payload["status"] = "error"
        payload["error"] = message
        return 2, payload
