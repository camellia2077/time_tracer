import re
from collections import OrderedDict
from pathlib import Path

from .config import LANG_CHOICES, ComponentConfig
from .guidance import guidance_for, guidance_payload


MANY_LARGE_FILES_THRESHOLD = 5
MANY_SOURCE_FILES_THRESHOLD = 100
TEST_HEAVY_RATIO = 0.4
TEST_LIGHT_RATIO = 0.1
HIGH_HOTSPOT_CONCENTRATION_RATIO = 0.4
_RUST_TEST_CFG_PATTERN = re.compile(r"^\s*#\s*\[\s*cfg\s*\(\s*test\s*\)\s*\]")

_WINDOWS_CLI_ZONES = (
    ("cli_models", "windows/rust/src/cli"),
    ("command_handlers", "windows/rust/src/commands"),
    ("runtime_boundary", "windows/rust/src/core/runtime"),
    ("error_surface", "windows/rust/src/error"),
)
_BOUNDARY_ASSESSMENT_CODES = {
    "CLI_CROSS_LAYER_HOTSPOTS",
    "CLI_HANDLER_FAMILY_HOTSPOTS",
    "CLI_RUNTIME_BOUNDARY_HOTSPOT",
}


def build_component_assessment(
    *,
    component: ComponentConfig,
    component_root: Path,
    files: list[dict],
    test_evidence: dict | None = None,
) -> dict:
    """Return explainable architecture signals for a known component.

    These are review prompts, not refactoring decisions. Keep component-specific
    rules here so generic LOC labels remain useful for every other component.
    """
    if component.name != "windows_cli":
        return {"kind": "generic", "signals": []}

    layers = {
        name: {"files": 0, "lines": 0, "hotspots": 0}
        for name, _prefix in _WINDOWS_CLI_ZONES
    }
    layers["other"] = {"files": 0, "lines": 0, "hotspots": 0}
    hotspot_handler_families: set[str] = set()

    for item in files:
        try:
            relative_path = Path(item["path"]).resolve().relative_to(component_root.resolve())
        except ValueError:
            relative_path = Path(item["path"])
        normalized_path = relative_path.as_posix().lower()
        zone = "other"
        for zone_name, prefix in _WINDOWS_CLI_ZONES:
            if normalized_path == prefix or normalized_path.startswith(prefix + "/"):
                zone = zone_name
                break
        layer = layers[zone]
        layer["files"] += 1
        layer["lines"] += item["lines"]
        if item["matched"]:
            layer["hotspots"] += 1
            if zone == "command_handlers":
                parts = normalized_path.split("/")
                try:
                    handler_index = parts.index("handlers")
                except ValueError:
                    handler_index = -1
                if handler_index >= 0 and len(parts) > handler_index + 1:
                    hotspot_handler_families.add(Path(parts[handler_index + 1]).stem)

    hotspot_layers = [name for name, data in layers.items() if data["hotspots"]]
    signals: list[dict[str, str]] = []
    if len(hotspot_layers) >= 2:
        signals.append({
            "code": "CLI_CROSS_LAYER_HOTSPOTS",
            "reason": (
                f"Hotspots span {len(hotspot_layers)} CLI ownership zones: "
                + ", ".join(hotspot_layers)
                + "; inspect boundaries before proposing a split."
            ),
        })
    if len(hotspot_handler_families) >= 2:
        signals.append({
            "code": "CLI_HANDLER_FAMILY_HOTSPOTS",
            "reason": (
                f"Hotspots occur in {len(hotspot_handler_families)} command handler families: "
                + ", ".join(sorted(hotspot_handler_families))
                + "; compare their reasons to change before sharing helpers."
            ),
        })
    if layers["runtime_boundary"]["hotspots"]:
        signals.append({
            "code": "CLI_RUNTIME_BOUNDARY_HOTSPOT",
            "reason": (
                "A Runtime boundary file is a hotspot; preserve one ABI transport seam "
                "and keep semantic work in Core."
            ),
        })
    test_evidence = test_evidence or {}
    inline_tests = test_evidence.get("inline_tests", {})
    test_support = test_evidence.get("test_support", {})
    if inline_tests.get("files"):
        signals.append({
            "code": "CLI_INLINE_TESTS_PRESENT",
            "reason": (
                f"{inline_tests['files']} Rust files contain cfg(test) modules "
                f"({inline_tests['lines']} estimated lines); keep production and test "
                "responsibilities separate when evaluating file size."
            ),
        })
    if test_support.get("files"):
        signals.append({
            "code": "CLI_TEST_SUPPORT_PRESENT",
            "reason": (
                f"{test_support['files']} CLI test-support files account for "
                f"{test_support['lines']} lines and should not be treated as production "
                "command behavior."
            ),
        })
    if not any(item["category"] == "tests" for item in files) and not (
        inline_tests.get("files") or test_support.get("files")
    ):
        signals.append({
            "code": "CLI_NO_DISCOVERED_TEST_EVIDENCE",
            "reason": (
                "No path-based or inline CLI test evidence was discovered; validate "
                "behavior with the Windows CLI black-box suite and Core/runtime "
                "contract tests."
            ),
        })

    return {
        "kind": "windows_cli",
        "signals": signals,
        "layers": layers,
        "hotspot_layers": hotspot_layers,
        "hotspot_handler_families": sorted(hotspot_handler_families),
        "test_evidence": test_evidence,
    }


def _rust_inline_test_line_count(file_path: Path) -> int:
    try:
        lines = file_path.read_text(encoding="utf-8", errors="replace").splitlines()
    except (OSError, UnicodeError):
        return 0

    test_lines: set[int] = set()
    for marker_index, line in enumerate(lines):
        if not _RUST_TEST_CFG_PATTERN.match(line):
            continue
        depth = 0
        opened = False
        end_index = len(lines) - 1
        for index in range(marker_index, len(lines)):
            line_without_comment = lines[index].split("//", 1)[0]
            opening = line_without_comment.count("{")
            closing = line_without_comment.count("}")
            if opening:
                opened = True
            depth += opening - closing
            if opened and depth <= 0:
                end_index = index
                break
        test_lines.update(range(marker_index, end_index + 1))
    return len(test_lines)


def build_test_evidence(*, component: ComponentConfig, files: list[dict]) -> dict:
    inline_test_files = 0
    inline_test_lines = 0
    test_support_files = 0
    test_support_lines = 0
    for item in files:
        file_path = Path(item["path"])
        if item.get("lang") == "rs":
            inline_lines = _rust_inline_test_line_count(file_path)
            if inline_lines:
                inline_test_files += 1
                inline_test_lines += inline_lines
        normalized_path = file_path.as_posix().lower()
        if (
            component.name == "windows_cli"
            and normalized_path.endswith("windows/rust/src/commands/testing.rs")
        ):
            test_support_files += 1
            test_support_lines += item["lines"]
    return {
        "inline_tests": {
            "files": inline_test_files,
            "lines": inline_test_lines,
        },
        "test_support": {
            "files": test_support_files,
            "lines": test_support_lines,
        },
    }


def attach_guidance(groups: list[dict]) -> None:
    for group in groups:
        guidance = guidance_for(group["component"], group["category"])
        group["guidance"] = guidance_payload(guidance)
        for language_result in group["languages"]:
            for file_result in language_result["matched_files"]:
                file_result["guidance"] = guidance_payload(guidance)


def build_language_summary(*, lang: str, files: list[dict]) -> dict:
    source_sets = {
        category: {
            "files": sum(item["category"] == category for item in files),
            "lines": sum(
                item["lines"]
                for item in files
                if item["category"] == category
            ),
        }
        for category in ("production", "tests", "other")
    }
    top_files = sorted(
        files,
        key=lambda item: (-item["lines"], item["path"].casefold()),
    )[:5]
    return {
        "lang": lang,
        "files": len(files),
        "lines": sum(item["lines"] for item in files),
        "matched_files": sum(item["matched"] for item in files),
        "matched_lines": sum(item["lines"] for item in files if item["matched"]),
        "source_sets": source_sets,
        "top_files": [
            {
                "path": item["path"],
                "lines": item["lines"],
                "category": item["category"],
                "matched": item["matched"],
            }
            for item in top_files
        ],
    }


def build_module_summary(
    *,
    component: ComponentConfig,
    component_root: Path,
    language_summaries: list[dict],
    files: list[dict],
    scan_settings: dict[str, dict[str, int | str]],
    external_test_roots: tuple[str, ...] = (),
) -> dict:
    source_sets = {
        category: {
            "files": sum(item["category"] == category for item in files),
            "lines": sum(
                item["lines"]
                for item in files
                if item["category"] == category
            ),
        }
        for category in ("production", "tests", "other")
    }
    source_directories: set[str] = set()
    max_source_depth = 0
    for item in files:
        file_path = Path(item["path"])
        try:
            relative_parent = file_path.parent.relative_to(component_root)
        except ValueError:
            relative_parent = file_path.parent
        source_directories.add(str(relative_parent) if str(relative_parent) else ".")
        max_source_depth = max(max_source_depth, len(relative_parent.parts))

    top_files = sorted(
        files,
        key=lambda item: (-item["lines"], item["path"].casefold()),
    )[:5]
    total_files = len(files)
    total_lines = sum(item["lines"] for item in files)
    matched_files = sum(item["matched"] for item in files)
    matched_lines = sum(item["lines"] for item in files if item["matched"])
    test_evidence = build_test_evidence(component=component, files=files)
    assessment = build_component_assessment(
        component=component,
        component_root=component_root,
        files=files,
        test_evidence=test_evidence,
    )
    test_evidence_files = (
        source_sets["tests"]["files"]
        + test_evidence["inline_tests"]["files"]
    )
    test_evidence_lines = (
        source_sets["tests"]["lines"]
        + test_evidence["inline_tests"]["lines"]
    )
    test_evidence_file_ratio = (
        test_evidence_files / total_files if total_files else 0.0
    )
    test_evidence_line_ratio = (
        test_evidence_lines / total_lines if total_lines else 0.0
    )
    test_file_ratio = (
        source_sets["tests"]["files"] / total_files if total_files else 0.0
    )
    test_line_ratio = (
        source_sets["tests"]["lines"] / total_lines if total_lines else 0.0
    )
    hotspot_line_ratio = matched_lines / total_lines if total_lines else 0.0
    labels, label_reasons = build_module_labels(
        scan_settings=scan_settings,
        total_files=total_files,
        matched_files=matched_files,
        matched_lines=matched_lines,
        total_lines=total_lines,
        test_file_ratio=test_file_ratio,
        test_line_ratio=test_line_ratio,
        source_sets=source_sets,
        external_test_roots=external_test_roots,
        assessment=assessment,
        test_evidence=test_evidence,
        label_test_file_ratio=test_evidence_file_ratio,
        label_test_line_ratio=test_evidence_line_ratio,
    )
    return {
        "component": component.name,
        "display_name": component.display_name,
        "root": str(component_root),
        "category": component.category,
        "priority": component.priority,
        "files": total_files,
        "lines": total_lines,
        "matched_files": matched_files,
        "matched_lines": matched_lines,
        "scan": {"languages": scan_settings},
        "ratios": {
            "test_files": round(test_file_ratio, 4),
            "test_lines": round(test_line_ratio, 4),
            "test_evidence_files": round(
                test_evidence_file_ratio, 4
            ),
            "test_evidence_lines": round(
                test_evidence_line_ratio, 4
            ),
            "hotspot_lines": round(hotspot_line_ratio, 4),
        },
        "labels": labels,
        "label_reasons": label_reasons,
        "source_sets": source_sets,
        "source_directories": len(source_directories),
        "max_source_depth": max_source_depth,
        "test_roots": list(external_test_roots),
        "test_evidence": test_evidence,
        "assessment": assessment,
        "languages": language_summaries,
        "top_files": [
            {
                "path": item["path"],
                "lines": item["lines"],
                "category": item["category"],
                "language": item["lang"],
                "matched": item["matched"],
            }
            for item in top_files
        ],
    }


def build_module_labels(
    *,
    scan_settings: dict[str, dict[str, int | str]],
    total_files: int,
    matched_files: int,
    matched_lines: int,
    total_lines: int,
    test_file_ratio: float,
    test_line_ratio: float,
    source_sets: dict,
    external_test_roots: tuple[str, ...] = (),
    assessment: dict | None = None,
    test_evidence: dict | None = None,
    label_test_file_ratio: float | None = None,
    label_test_line_ratio: float | None = None,
) -> tuple[list[str], dict[str, str]]:
    labels: list[str] = []
    reasons: dict[str, str] = {}
    label_test_file_ratio = (
        test_file_ratio if label_test_file_ratio is None else label_test_file_ratio
    )
    label_test_line_ratio = (
        test_line_ratio if label_test_line_ratio is None else label_test_line_ratio
    )

    def add(label: str, reason: str) -> None:
        labels.append(label)
        reasons[label] = reason

    scan_modes = {str(setting["mode"]) for setting in scan_settings.values()}
    if scan_modes == {"over"}:
        thresholds = {int(setting["threshold"]) for setting in scan_settings.values()}
        threshold_text = (
            str(next(iter(thresholds)))
            if len(thresholds) == 1
            else "language defaults"
        )
        if matched_files:
            add("LARGE_FILE", f"{matched_files} files meet or exceed the {threshold_text}-line threshold.")
        if matched_files >= MANY_LARGE_FILES_THRESHOLD:
            add("MANY_LARGE_FILES", f"{matched_files} files meet or exceed the threshold.")
        hotspot_line_ratio = matched_lines / total_lines if total_lines else 0.0
        if hotspot_line_ratio >= HIGH_HOTSPOT_CONCENTRATION_RATIO:
            add(
                "HIGH_HOTSPOT_CONCENTRATION",
                f"Files meeting the threshold account for {hotspot_line_ratio:.1%} of module LOC.",
            )
    elif scan_modes == {"under"} and matched_files >= MANY_LARGE_FILES_THRESHOLD:
        thresholds = {int(setting["threshold"]) for setting in scan_settings.values()}
        threshold_text = (
            str(next(iter(thresholds)))
            if len(thresholds) == 1
            else "language defaults"
        )
        add(
            "SMALL_FILE_CLUSTER",
            f"{matched_files} files are below the {threshold_text}-line threshold.",
        )

    if total_files >= MANY_SOURCE_FILES_THRESHOLD:
        add("MANY_SOURCE_FILES", f"Module contains {total_files} scannable source files.")
    if source_sets["other"]["files"]:
        add(
            "OTHER_SOURCE_SET_PRESENT",
            f"{source_sets['other']['files']} files are outside production/tests source sets.",
        )
    if external_test_roots and source_sets["tests"]["files"]:
        add(
            "TESTS_EXTERNAL",
            "Test sources were counted from configured external test roots: "
            + ", ".join(external_test_roots),
        )
    elif (
        source_sets["tests"]["files"] == 0
        and source_sets["production"]["files"]
        and not (
            (test_evidence or {}).get("inline_tests", {}).get("files")
            or (test_evidence or {}).get("test_support", {}).get("files")
        )
        and (assessment or {}).get("kind") != "windows_cli"
    ):
        add("NO_TESTS", "Module has production code but no test source files were found.")
    elif (
        label_test_file_ratio >= TEST_HEAVY_RATIO
        or label_test_line_ratio >= TEST_HEAVY_RATIO
    ):
        add(
            "TEST_HEAVY",
            f"Test evidence share is high: {label_test_file_ratio:.1%} of files and {label_test_line_ratio:.1%} of LOC.",
        )
    elif (
        source_sets["tests"]["files"]
        and label_test_file_ratio < TEST_LIGHT_RATIO
        and label_test_line_ratio < TEST_LIGHT_RATIO
    ):
        add(
            "TEST_LIGHT",
            f"Test evidence share is low: {label_test_file_ratio:.1%} of files and {label_test_line_ratio:.1%} of LOC.",
        )

    for signal in (assessment or {}).get("signals", []):
        add(signal["code"], signal["reason"])

    return labels, reasons


def build_module_reading_candidates(module_summaries: list[dict]) -> list[dict]:
    """Build suggested reading candidates, not mandatory refactoring order."""
    module_summaries = [module for module in module_summaries if module["files"]]
    if not module_summaries:
        return []

    max_lines = max(module["lines"] for module in module_summaries) or 1
    max_hotspots = max(module["matched_files"] for module in module_summaries) or 1
    rankings: list[dict] = []

    for module in module_summaries:
        architecture_score = 60 * max(0, 4 - module["priority"]) / 4
        scale_score = 20 * module["lines"] / max_lines
        hotspot_score = 15 * module["matched_files"] / max_hotspots
        assessment_codes = {
            signal["code"]
            for signal in module.get("assessment", {}).get("signals", [])
        }
        boundary_score = 5 if (
            "OTHER_SOURCE_SET_PRESENT" in module["labels"]
            or assessment_codes & _BOUNDARY_ASSESSMENT_CODES
        ) else 0
        score = architecture_score + scale_score + hotspot_score + boundary_score
        production = module["source_sets"]["production"]
        tests = module["source_sets"]["tests"]
        reasons = [
            f"P{module['priority']} {module['category']} priority",
            f"Scale: {module['lines']} lines / {module['files']} files",
            f"Hotspots: {module['matched_files']} files",
        ]
        if tests["files"]:
            test_lines = module.get("ratios", {}).get(
                "test_evidence_lines",
                tests["lines"] / module["lines"] if module["lines"] else 0,
            )
            test_evidence = module.get("test_evidence", {})
            inline_lines = test_evidence.get("inline_tests", {}).get("lines", 0)
            total_test_lines = tests["lines"] + inline_lines
            reasons.append(
                f"Test evidence: {total_test_lines} lines ({test_lines:.1%})"
            )
        elif module.get("test_evidence", {}).get("inline_tests", {}).get("files"):
            test_lines = module.get("ratios", {}).get("test_evidence_lines", 0.0)
            inline_lines = module["test_evidence"]["inline_tests"]["lines"]
            reasons.append(f"Test evidence: {inline_lines} lines ({test_lines:.1%})")
        if module["labels"]:
            reasons.append(f"Labels: {', '.join(module['labels'])}")
        assessment_signals = module.get("assessment", {}).get("signals", [])
        if assessment_signals:
            reasons.append(
                "Assessment: "
                + ", ".join(signal["code"] for signal in assessment_signals)
            )
        rankings.append(
            {
                "component": module["component"],
                "display_name": module["display_name"],
                "category": module["category"],
                "priority": module["priority"],
                "reading_score": round(score, 2),
                "reading_score_breakdown": {
                    "architecture_priority": round(architecture_score, 2),
                    "module_scale": round(scale_score, 2),
                    "hotspot_volume": round(hotspot_score, 2),
                    "boundary_signal": round(boundary_score, 2),
                },
                "reasons": reasons,
                "production_lines": production["lines"],
                "test_lines": tests["lines"],
                "matched_files": module["matched_files"],
            }
        )

    rankings.sort(
        key=lambda item: (
            -item["reading_score"],
            item["priority"],
            -item["matched_files"],
            item["display_name"].casefold(),
        )
    )
    for rank, item in enumerate(rankings, start=1):
        item["reading_rank"] = rank
    return rankings


def normalize_profile_groups(
    grouped_results: OrderedDict[tuple[str, str], dict],
) -> list[dict]:
    category_order = {"libs": 0, "presentation": 1, "tests": 2}
    groups = []
    for group in sorted(
        grouped_results.values(),
        key=lambda item: (
            category_order.get(item["category"], 99),
            item["category"],
            item["component"],
        ),
    ):
        group["languages"] = [
            language_result
            for language_result in sorted(
                group["languages"].values(),
                key=lambda item: LANG_CHOICES.index(item["lang"]),
            )
        ]
        for language_result in group["languages"]:
            language_result["matched_files"].sort(
                key=lambda item: item["lines"],
                reverse=language_result["mode"] == "over",
            )
        groups.append(group)
    return groups


def build_priority_groups(groups: list[dict]) -> list[dict]:
    findings: list[dict] = []
    for group in groups:
        for language_result in group["languages"]:
            for file_result in language_result["matched_files"]:
                findings.append(
                    {
                        "priority": group["priority"],
                        "category": group["category"],
                        "component": group["component"],
                        "display_name": group["display_name"],
                        "language": language_result["lang"],
                        "path": file_result["path"],
                        "lines": file_result["lines"],
                        "guidance": file_result.get("guidance", {}),
                    }
                )

    findings.sort(
        key=lambda item: (
            item["priority"],
            -item["lines"],
            item["path"].casefold(),
        )
    )
    priority_groups: OrderedDict[int, dict] = OrderedDict()
    for finding in findings:
        priority_group = priority_groups.setdefault(
            finding["priority"],
            {"priority": finding["priority"], "findings": []},
        )
        priority_group["findings"].append(finding)
    return list(priority_groups.values())
