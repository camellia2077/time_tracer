import json
from collections import OrderedDict
from pathlib import Path

from .classification import resolve_component_root, resolve_profile_baseline_category, resolve_result_category
from .config import LANG_CHOICES, load_language_config, load_scan_profile
from .profile_analysis import attach_guidance, build_language_summary, build_module_reading_candidates, build_module_summary, build_priority_groups, normalize_profile_groups
from .reporter import LocConsoleReporter
from .report_writer import save_profile_baseline, write_profile_comparison_report, write_profile_report
from .service import LocScanService, ScanArgumentResolver


class ProfileScanRunner:
    def __init__(self, *, workspace_root: Path, config_path: Path):
        self.workspace_root = workspace_root
        self.config_path = config_path

    def run(self, args, payload: dict) -> tuple[int, dict]:
        if args.paths:
            return self._error("--profile mode does not accept paths; configure component roots in the TOML profile.", payload)
        if args.dir_over_files is not None:
            return self._error("--profile mode does not support --dir-over-files; use --lang mode.", payload)
        try:
            profile = load_scan_profile(self.config_path, args.profile)
            language_configs = {lang: load_language_config(self.config_path, lang) for lang in LANG_CHOICES}
        except (FileNotFoundError, ValueError, OSError) as error:
            return self._error(f"Configuration loading failed: {error}", payload)

        baseline_path = self._resolve_optional_path(args.compare_baseline)
        if args.compare_baseline and not baseline_path.exists():
            return self._error(f"Baseline file does not exist: {baseline_path}", payload)
        save_baseline_path = self._resolve_optional_path(args.save_baseline)
        resolver = ScanArgumentResolver()
        grouped_results: OrderedDict[tuple[str, str], dict] = OrderedDict()
        missing_paths: list[Path] = []
        module_summaries: list[dict] = []

        for component in profile.components:
            component_root = resolve_component_root(component, self.workspace_root)
            if not component_root.exists():
                missing_paths.append(component_root)
                continue
            grouped_results.setdefault((component.category, component.name), {
                "category": component.category, "component": component.name,
                "display_name": component.display_name, "root": str(component_root),
                "priority": component.priority, "languages": OrderedDict(),
            })
            language_summaries: list[dict] = []
            module_files: list[dict] = []
            scan_settings: dict[str, dict[str, int | str]] = {}
            excluded_roots = tuple(
                (self.workspace_root / root).resolve()
                if not Path(root).is_absolute()
                else Path(root).resolve()
                for root in component.exclude_roots
            )
            for lang in profile.languages:
                language_config = language_configs[lang]
                mode, threshold = resolver.resolve_mode_and_threshold(args, language_config)
                if threshold <= 0:
                    return self._error(f"{lang} threshold must be a positive integer.", payload)
                scan_settings[lang] = {"mode": mode, "threshold": threshold}
                scan_service = LocScanService(language_config)
                all_files = scan_service.scan_files(
                    component_root,
                    excluded_roots=excluded_roots,
                )
                classified_files = [
                    {
                        "path": file_path, "lines": lines, "lang": lang,
                        "category": resolve_profile_baseline_category(
                            file_path=Path(file_path), scan_root=component_root,
                            component=component, lang=lang,
                            test_directory_names=profile.test_directory_names,
                        ),
                        "matched": False,
                    }
                    for file_path, lines in all_files
                ]
                external_test_files = self._scan_external_test_files(
                    component=component,
                    component_root=component_root,
                    scan_service=scan_service,
                    lang=lang,
                    mode=mode,
                    threshold=threshold,
                )
                known_paths = {str(Path(item["path"]).resolve()) for item in classified_files}
                classified_files.extend(
                    item for item in external_test_files
                    if str(Path(item["path"]).resolve()) not in known_paths
                )
                matched_paths = {
                    file_path
                    for file_path, _ in scan_service.filter_files(all_files, mode, threshold)
                }
                classified_files = [
                    {**item, "matched": item["matched"] or item["path"] in matched_paths}
                    for item in classified_files
                ]
                language_summaries.append(build_language_summary(lang=lang, files=classified_files))
                module_files.extend(classified_files)
                self._append_matched_files(
                    grouped_results=grouped_results, classified_files=classified_files,
                    component=component, component_root=component_root, profile=profile,
                    language=lang, mode=mode, threshold=threshold,
                    over_inclusive=language_config.over_inclusive,
                )
            module_summaries.append(build_module_summary(
                component=component, component_root=component_root,
                language_summaries=language_summaries, files=module_files,
                scan_settings=scan_settings,
                external_test_roots=component.test_roots,
            ))

        groups = normalize_profile_groups(grouped_results)
        attach_guidance(groups)
        priority_groups = build_priority_groups(groups)
        module_reading_candidates = build_module_reading_candidates(module_summaries)
        summary = {
            "matched_files": sum(len(language_result["matched_files"])
                for group in groups for language_result in group["languages"]),
            "modules": len(module_summaries),
        }
        scan = {
            "mode": self._resolve_profile_mode(args),
            "profile": profile.name,
            "languages": list(profile.languages),
        }
        report_paths = write_profile_report(
            workspace_root=self.workspace_root, profile_name=profile.name,
            profile_display_name=profile.display_name, scan=scan, summary=summary,
            groups=groups, priority_groups=priority_groups,
            missing_paths=missing_paths, module_summaries=module_summaries,
            module_reading_candidates=module_reading_candidates,
        )
        current_report_path = Path(report_paths["json"]).resolve()
        if save_baseline_path is not None:
            if save_baseline_path == current_report_path:
                return self._error("--save-baseline cannot overwrite the current scan report; use a separate path.", payload)
            report_paths["baseline_json"] = save_profile_baseline(
                current_path=current_report_path, baseline_path=save_baseline_path,
            )
        if baseline_path is not None:
            if baseline_path == current_report_path:
                return self._error("--compare-baseline must point to a previously saved independent JSON report.", payload)
            try:
                comparison_paths = write_profile_comparison_report(
                    workspace_root=self.workspace_root, profile_name=profile.name,
                    baseline_path=baseline_path, current_path=current_report_path,
                )
            except (OSError, ValueError, json.JSONDecodeError) as error:
                return self._error(f"Baseline comparison failed: {error}", payload)
            report_paths["comparison_markdown"] = comparison_paths["markdown"]
            report_paths["comparison_json"] = comparison_paths["json"]
            payload["comparison"] = comparison_paths["summary"]
        LocConsoleReporter(language_configs["cpp"]).print_profile_scan_report(
            profile=profile, mode=self._resolve_profile_mode(args), groups=groups,
            priority_groups=priority_groups, missing_paths=missing_paths,
            report_paths=report_paths, module_summaries=module_summaries,
            module_reading_candidates=module_reading_candidates,
        )
        payload.update({
            "status": "ok", "scan": scan, "results": groups,
            "priority_results": priority_groups,
            "module_summaries": module_summaries,
            "module_reading_candidates": module_reading_candidates,
            "summary": summary, "report": report_paths,
        })
        if missing_paths:
            payload["missing_paths"] = [str(path) for path in missing_paths]
        return 0, payload

    def _scan_external_test_files(
        self,
        *,
        component,
        component_root: Path,
        scan_service: LocScanService,
        lang: str,
        mode: str,
        threshold: int,
    ) -> list[dict]:
        files: list[dict] = []
        seen: set[Path] = set()
        for raw_root in component.test_roots:
            root = Path(raw_root)
            if not root.is_absolute():
                root = self.workspace_root / root
            root = root.resolve()
            if not root.exists():
                continue
            for file_path, lines in scan_service.scan_files(root):
                resolved = Path(file_path).resolve()
                if resolved in seen:
                    continue
                seen.add(resolved)
                files.append({
                    "path": file_path,
                    "lines": lines,
                    "lang": lang,
                    "category": "tests",
                    "matched": scan_service._matches_threshold(lines, mode, threshold),
                })
        return files

    @staticmethod
    def _append_matched_files(
        *, grouped_results, classified_files, component, component_root,
        profile, language, mode, threshold, over_inclusive,
    ) -> None:
        for file_result in classified_files:
            if not file_result["matched"]:
                continue
            category = resolve_result_category(
                file_path=Path(file_result["path"]),
                component_root=component_root,
                component=component,
                test_directory_names=profile.test_directory_names,
            )
            priority = profile.test_priority if category == "tests" else component.priority
            group = grouped_results.setdefault((category, component.name), {
                "category": category, "component": component.name,
                "display_name": component.display_name, "root": str(component_root),
                "priority": priority, "languages": OrderedDict(),
            })
            language_result = group["languages"].setdefault(language, {
                "lang": language, "mode": mode, "threshold": threshold,
                "over_inclusive": over_inclusive, "priority": priority,
                "matched_files": [],
            })
            language_result["matched_files"].append({
                "path": file_result["path"],
                "lines": file_result["lines"],
                "priority": priority,
            })

    def _resolve_optional_path(self, raw_path: str | None) -> Path | None:
        if not raw_path:
            return None
        path = Path(raw_path)
        if not path.is_absolute():
            path = self.workspace_root / path
        return path.resolve()

    @staticmethod
    def _resolve_profile_mode(args) -> str:
        return "under" if args.under is not None else "over"

    @staticmethod
    def _error(message: str, payload: dict) -> tuple[int, dict]:
        print(f"[ERROR] {message}")
        payload["status"] = "error"
        payload["error"] = message
        return 2, payload
