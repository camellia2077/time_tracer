import argparse
import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC_ROOT = PROJECT_ROOT / "src"
if str(SRC_ROOT) not in sys.path:
    sys.path.insert(0, str(SRC_ROOT))

from loc_scanner.cli_app import LocCliApplication
from loc_scanner.classification import (
    resolve_profile_baseline_category,
    resolve_result_category,
    resolve_single_language_category,
)
from loc_scanner.config import ComponentConfig, load_language_config
from loc_scanner.guidance import guidance_for
from loc_scanner.profile_analysis import (
    build_language_summary,
    build_module_reading_candidates,
    build_module_summary,
    build_priority_groups,
)
from loc_scanner.report_writer import write_profile_report
from loc_scanner.service import LocScanService, ScanArgumentResolver


def test_resolve_paths_with_workspace_root(tmp_path: Path) -> None:
    resolver = ScanArgumentResolver()
    workspace_root = tmp_path / "workspace"
    workspace_root.mkdir(parents=True, exist_ok=True)

    paths = resolver.resolve_paths(
        raw_paths=["src", str(tmp_path)],
        default_paths=["."],
        path_mode="cli_override",
        workspace_root=workspace_root,
    )

    assert paths[0] == (workspace_root / "src").resolve()
    assert paths[1] == tmp_path.resolve()


def test_resolve_paths_toml_only_ignores_cli_paths(tmp_path: Path) -> None:
    resolver = ScanArgumentResolver()
    workspace_root = tmp_path / "workspace"
    workspace_root.mkdir(parents=True, exist_ok=True)

    paths = resolver.resolve_paths(
        raw_paths=["libs", "apps"],
        default_paths=["apps", "libs"],
        path_mode="toml_only",
        workspace_root=workspace_root,
    )

    assert paths == [
        (workspace_root / "apps").resolve(),
        (workspace_root / "libs").resolve(),
    ]


def test_resolve_paths_merge_combines_and_deduplicates(tmp_path: Path) -> None:
    resolver = ScanArgumentResolver()
    workspace_root = tmp_path / "workspace"
    workspace_root.mkdir(parents=True, exist_ok=True)

    paths = resolver.resolve_paths(
        raw_paths=["libs", "tools"],
        default_paths=["apps", "libs"],
        path_mode="merge",
        workspace_root=workspace_root,
    )

    assert paths == [
        (workspace_root / "apps").resolve(),
        (workspace_root / "libs").resolve(),
        (workspace_root / "tools").resolve(),
    ]


def test_scan_files_skips_excluded_component_roots(tmp_path: Path) -> None:
    source_root = tmp_path / "tools" / "toolchain"
    tidy_root = source_root / "commands" / "tidy"
    source_root.mkdir(parents=True)
    tidy_root.mkdir(parents=True)
    (source_root / "glue.py").write_text("print('glue')\n", encoding="utf-8")
    (tidy_root / "workflow.py").write_text("print('tidy')\n", encoding="utf-8")

    config_path = PROJECT_ROOT / "config" / "scan_lines.toml"
    config = load_language_config(config_path=config_path, lang="py")
    files = LocScanService(config).scan_files(
        source_root,
        excluded_roots=(tidy_root,),
    )

    assert [Path(path).name for path, _lines in files] == ["glue.py"]


def test_default_log_path_uses_workspace_root(tmp_path: Path) -> None:
    log_path = LocCliApplication._resolve_log_path(
        None,
        "py",
        workspace_root=tmp_path,
    )
    expected = (tmp_path / "temp" / "loc_scanner" / "logs" / "scan_py.json").resolve()
    assert log_path == expected


def test_profile_log_path_uses_profile_name(tmp_path: Path) -> None:
    log_path = LocCliApplication._resolve_log_path(
        None,
        None,
        profile="core_family",
        workspace_root=tmp_path,
    )
    expected = (
        tmp_path / "temp" / "loc_scanner" / "logs" / "scan_profile_core_family.json"
    ).resolve()
    assert log_path == expected


def test_profile_classifies_test_paths_separately(tmp_path: Path) -> None:
    component = ComponentConfig(
        name="android",
        display_name="apps/android",
        root="apps/android",
        category="presentation",
        priority=2,
    )
    component_root = (tmp_path / "apps" / "android").resolve()

    assert (
        resolve_result_category(
            file_path=component_root / "feature" / "src" / "main" / "Main.kt",
            component_root=component_root,
            component=component,
            test_directory_names=("test", "tests", "androidtest"),
        )
        == "presentation"
    )
    assert (
        resolve_result_category(
            file_path=component_root / "feature" / "src" / "test" / "MainTest.kt",
            component_root=component_root,
            component=component,
            test_directory_names=("test", "tests", "androidtest"),
        )
        == "tests"
    )


def test_single_language_classifies_android_source_sets(tmp_path: Path) -> None:
    scan_root = (tmp_path / "apps" / "android").resolve()
    test_directory_names = ("test", "tests", "androidtest", "testfixtures")

    assert (
        resolve_single_language_category(
            file_path=scan_root / "feature" / "src" / "main" / "Main.kt",
            scan_root=scan_root,
            test_directory_names=test_directory_names,
        )
        == "production"
    )
    assert (
        resolve_single_language_category(
            file_path=scan_root / "feature" / "src" / "test" / "MainTest.kt",
            scan_root=scan_root,
            test_directory_names=test_directory_names,
        )
        == "tests"
    )
    assert (
        resolve_single_language_category(
            file_path=scan_root / "buildSrc" / "Tool.kt",
            scan_root=scan_root,
            test_directory_names=test_directory_names,
        )
        == "other"
    )


def test_profile_baseline_treats_non_android_non_test_code_as_production(
    tmp_path: Path,
) -> None:
    component = ComponentConfig(
        name="tracer_core",
        display_name="libs/tracer_core",
        root="libs/tracer_core",
        category="libs",
        priority=0,
    )
    component_root = (tmp_path / "libs" / "tracer_core").resolve()

    assert (
        resolve_profile_baseline_category(
            file_path=component_root / "src" / "infra" / "config.cpp",
            scan_root=component_root,
            component=component,
            lang="cpp",
            test_directory_names=("test", "tests"),
        )
        == "production"
    )


def test_windows_cli_test_support_is_classified_as_tests(tmp_path: Path) -> None:
    component = ComponentConfig(
        name="windows_cli",
        display_name="apps/cli",
        root="apps/cli",
        category="presentation",
        priority=2,
    )
    component_root = (tmp_path / "apps" / "cli").resolve()

    assert (
        resolve_profile_baseline_category(
            file_path=component_root / "windows" / "rust" / "src" / "commands" / "testing.rs",
            scan_root=component_root,
            component=component,
            lang="rs",
            test_directory_names=("test", "tests"),
        )
        == "tests"
    )


def test_module_summary_contains_baseline_counts_and_top_files(tmp_path: Path) -> None:
    component = ComponentConfig(
        name="android",
        display_name="apps/android",
        root="apps/android",
        category="presentation",
        priority=2,
    )
    component_root = (tmp_path / "apps" / "android").resolve()
    files = [
        {
            "path": str(component_root / "app" / "src" / "main" / "Main.kt"),
            "lines": 500,
            "lang": "kt",
            "category": "production",
            "matched": True,
        },
        {
            "path": str(component_root / "app" / "src" / "test" / "MainTest.kt"),
            "lines": 300,
            "lang": "kt",
            "category": "tests",
            "matched": False,
        },
    ]

    summary = build_module_summary(
        component=component,
        component_root=component_root,
        language_summaries=[
            build_language_summary(lang="kt", files=files)
        ],
        files=files,
        scan_settings={"kt": {"mode": "over", "threshold": 350}},
    )

    assert summary["files"] == 2
    assert summary["lines"] == 800
    assert summary["matched_files"] == 1
    assert summary["source_sets"]["production"] == {"files": 1, "lines": 500}
    assert summary["source_sets"]["tests"] == {"files": 1, "lines": 300}
    assert "LARGE_FILE" in summary["labels"]
    assert "TEST_HEAVY" in summary["labels"]
    assert "350-line threshold" in summary["label_reasons"]["LARGE_FILE"]
    assert summary["top_files"][0]["path"].endswith("Main.kt")


def test_windows_cli_assessment_reports_boundary_evidence(tmp_path: Path) -> None:
    component = ComponentConfig(
        name="windows_cli",
        display_name="apps/cli",
        root="apps/cli",
        category="presentation",
        priority=2,
    )
    component_root = (tmp_path / "apps" / "cli").resolve()
    files = [
        {
            "path": str(component_root / "windows" / "rust" / "src" / "cli" / "mod.rs"),
            "lines": 800,
            "lang": "rs",
            "category": "production",
            "matched": True,
        },
        {
            "path": str(component_root / "windows" / "rust" / "src" / "commands" / "handlers" / "alias.rs"),
            "lines": 1200,
            "lang": "rs",
            "category": "production",
            "matched": True,
        },
        {
            "path": str(component_root / "windows" / "rust" / "src" / "commands" / "handlers" / "report.rs"),
            "lines": 900,
            "lang": "rs",
            "category": "production",
            "matched": True,
        },
        {
            "path": str(component_root / "windows" / "rust" / "src" / "core" / "runtime" / "invoke.rs"),
            "lines": 700,
            "lang": "rs",
            "category": "production",
            "matched": True,
        },
    ]

    summary = build_module_summary(
        component=component,
        component_root=component_root,
        language_summaries=[build_language_summary(lang="rs", files=files)],
        files=files,
        scan_settings={"rs": {"mode": "over", "threshold": 350}},
    )

    signal_codes = [signal["code"] for signal in summary["assessment"]["signals"]]
    assert "CLI_CROSS_LAYER_HOTSPOTS" in signal_codes
    assert "CLI_HANDLER_FAMILY_HOTSPOTS" in signal_codes
    assert "CLI_RUNTIME_BOUNDARY_HOTSPOT" in signal_codes
    assert "CLI_NO_DISCOVERED_TEST_EVIDENCE" in signal_codes
    assert "NO_TESTS" not in summary["labels"]
    assert "CLI_CROSS_LAYER_HOTSPOTS" in summary["labels"]

    ranking = build_module_reading_candidates([summary])[0]
    assert ranking["reading_score_breakdown"]["boundary_signal"] == 5.0
    assert "Assessment: CLI_CROSS_LAYER_HOTSPOTS" in " ".join(ranking["reasons"])


def test_rust_inline_tests_are_reported_as_test_evidence(tmp_path: Path) -> None:
    component = ComponentConfig(
        name="windows_cli",
        display_name="apps/cli",
        root="apps/cli",
        category="presentation",
        priority=2,
    )
    component_root = (tmp_path / "apps" / "cli").resolve()
    source_path = component_root / "windows" / "rust" / "src" / "cli" / "mod.rs"
    source_path.parent.mkdir(parents=True)
    source_path.write_text(
        "pub struct Command;\n\n"
        "#[cfg(test)]\n"
        "mod tests {\n"
        "    #[test]\n"
        "    fn parses_command() { assert!(true); }\n"
        "}\n",
        encoding="utf-8",
    )
    files = [{
        "path": str(source_path),
        "lines": 8,
        "lang": "rs",
        "category": "production",
        "matched": False,
    }]

    summary = build_module_summary(
        component=component,
        component_root=component_root,
        language_summaries=[build_language_summary(lang="rs", files=files)],
        files=files,
        scan_settings={"rs": {"mode": "over", "threshold": 350}},
    )

    assert summary["test_evidence"]["inline_tests"]["files"] == 1
    assert summary["test_evidence"]["inline_tests"]["lines"] > 0
    assert summary["ratios"]["test_evidence_lines"] > 0
    signal_codes = [signal["code"] for signal in summary["assessment"]["signals"]]
    assert "CLI_INLINE_TESTS_PRESENT" in signal_codes
    assert "CLI_NO_DISCOVERED_TEST_EVIDENCE" not in signal_codes
    ranking = build_module_reading_candidates([summary])[0]
    assert ranking["reading_score_breakdown"]["boundary_signal"] == 0.0


def test_module_ranking_combines_priority_scale_and_hotspots() -> None:
    modules = [
        {
            "component": "core",
            "display_name": "libs/core",
            "category": "libs",
            "priority": 0,
            "files": 10,
            "lines": 1000,
            "matched_files": 1,
            "labels": [],
            "source_sets": {
                "production": {"files": 10, "lines": 1000},
                "tests": {"files": 0, "lines": 0},
                "other": {"files": 0, "lines": 0},
            },
        },
        {
            "component": "android",
            "display_name": "apps/android",
            "category": "presentation",
            "priority": 2,
            "files": 100,
            "lines": 5000,
            "matched_files": 20,
            "labels": ["MANY_LARGE_FILES"],
            "source_sets": {
                "production": {"files": 80, "lines": 4000},
                "tests": {"files": 20, "lines": 1000},
                "other": {"files": 0, "lines": 0},
            },
        },
    ]

    rankings = build_module_reading_candidates(modules)

    assert [item["display_name"] for item in rankings] == [
        "apps/android",
        "libs/core",
    ]
    assert rankings[0]["reading_rank"] == 1
    assert rankings[0]["reading_score_breakdown"]["hotspot_volume"] == 15.0
    assert "P2 presentation priority" in rankings[0]["reasons"]


def test_priority_groups_sort_by_priority_then_line_count() -> None:
    groups = [
        {
            "priority": 2,
            "category": "presentation",
            "component": "android",
            "display_name": "apps/android",
            "languages": [
                {
                    "lang": "kt",
                    "matched_files": [
                        {"path": "presentation.kt", "lines": 900},
                    ],
                }
            ],
        },
        {
            "priority": 0,
            "category": "libs",
            "component": "tracer_core",
            "display_name": "libs/tracer_core",
            "languages": [
                {
                    "lang": "cpp",
                    "matched_files": [
                        {"path": "core_small.cpp", "lines": 400},
                        {"path": "core_large.cpp", "lines": 1000},
                    ],
                }
            ],
        },
    ]

    priority_groups = build_priority_groups(groups)

    assert [group["priority"] for group in priority_groups] == [0, 2]
    assert [
        finding["lines"] for finding in priority_groups[0]["findings"]
    ] == [1000, 400]


def test_build_base_payload_contains_workspace_root(tmp_path: Path) -> None:
    args = argparse.Namespace(lang="py")
    payload = LocCliApplication._build_base_payload(args=args, workspace_root=tmp_path)
    assert payload["lang"] == "py"
    assert payload["workspace_root"] == str(tmp_path)


def test_write_json_log_creates_gitignore_for_log_dir(tmp_path: Path) -> None:
    log_path = tmp_path / "temp" / "loc_scanner" / "logs" / "scan_py.json"
    log_path.parent.mkdir(parents=True, exist_ok=True)

    LocCliApplication._write_json_log(log_path, {"status": "ok"})

    gitignore_path = log_path.parent / ".gitignore"
    assert gitignore_path.exists()
    assert (
        gitignore_path.read_text(encoding="utf-8")
        == "# Automatically created by loc_scanner.\n*\n"
    )


def test_write_json_log_preserves_existing_gitignore_and_appends_rules(tmp_path: Path) -> None:
    log_path = tmp_path / "temp" / "loc_scanner" / "logs" / "scan_py.json"
    log_path.parent.mkdir(parents=True, exist_ok=True)
    gitignore_path = log_path.parent / ".gitignore"
    gitignore_path.write_text("# keep custom rule\n", encoding="utf-8")

    LocCliApplication._write_json_log(log_path, {"status": "ok"})

    content = gitignore_path.read_text(encoding="utf-8")
    assert "# keep custom rule" in content
    assert "# Automatically created by loc_scanner." in content.splitlines()
    assert "*" in content.splitlines()


def test_guidance_is_selected_by_component_and_category() -> None:
    core_guidance = guidance_for("tracer_core", "libs")
    tests_guidance = guidance_for("shared_tests", "tests")
    android_tests_guidance = guidance_for("android", "tests")
    android_guidance = guidance_for("android", "presentation")
    tidy_guidance = guidance_for("tidy_workflow", "tools")
    clang_guidance = guidance_for("clang_adapters", "tools")

    assert core_guidance.path == "docs/time_tracer/architecture/refactoring_guidance.md"
    assert "capability" in core_guidance.summary
    assert tests_guidance.path is None
    assert "separate inventory" in tests_guidance.summary
    assert android_tests_guidance == tests_guidance
    assert android_guidance.path == "docs/time_tracer/architecture/refactoring_guidance.md"
    assert "Android" in android_guidance.summary
    assert "state transitions" in tidy_guidance.summary
    assert "tool invocation" in clang_guidance.summary


def test_external_test_roots_are_reported_separately() -> None:
    component = ComponentConfig(
        name="tidy_workflow",
        display_name="tools/toolchain/commands/tidy",
        root="tools/toolchain/commands/tidy",
        category="tools",
        priority=2,
        test_roots=("tools/tests/platform/tidy",),
    )
    summary = build_module_summary(
        component=component,
        component_root=Path("C:/repo/tools/toolchain/commands/tidy"),
        language_summaries=[],
        files=[
            {
                "path": "C:/repo/tools/toolchain/commands/tidy/source.py",
                "lines": 300,
                "lang": "py",
                "category": "production",
                "matched": True,
            },
            {
                "path": "C:/repo/tools/tests/platform/tidy/test_source.py",
                "lines": 120,
                "lang": "py",
                "category": "tests",
                "matched": False,
            },
        ],
        scan_settings={"py": {"mode": "over", "threshold": 200}},
        external_test_roots=component.test_roots,
    )

    assert "NO_TESTS" not in summary["labels"]
    assert "TESTS_EXTERNAL" in summary["labels"]
    assert summary["test_roots"] == ["tools/tests/platform/tidy"]


def test_profile_report_writes_markdown_and_json(tmp_path: Path) -> None:
    guidance = guidance_for("tracer_core", "libs")
    guidance_payload = {
        "summary": guidance.summary,
        "path": guidance.path,
        "validation": list(guidance.validation),
    }
    groups = [
        {
            "category": "libs",
            "component": "tracer_core",
            "display_name": "libs/tracer_core",
            "priority": 0,
            "languages": [
                {
                    "lang": "cpp",
                    "matched_files": [
                        {
                            "path": "libs/tracer_core/example.cpp",
                            "lines": 500,
                            "guidance": guidance_payload,
                        }
                    ],
                }
            ],
            "guidance": guidance_payload,
        }
    ]
    priority_groups = [
        {
            "priority": 0,
            "findings": [
                {
                    "category": "libs",
                    "component": "tracer_core",
                    "display_name": "libs/tracer_core",
                    "language": "cpp",
                    "path": "libs/tracer_core/example.cpp",
                    "lines": 500,
                    "guidance": guidance_payload,
                }
            ],
        }
    ]

    paths = write_profile_report(
        workspace_root=tmp_path,
        profile_name="core_family",
        profile_display_name="Core Family",
        scan={"mode": "over", "profile": "core_family"},
        summary={"matched_files": 1},
        groups=groups,
        priority_groups=priority_groups,
        missing_paths=[],
    )

    markdown = Path(paths["markdown"]).read_text(encoding="utf-8")
    report_json = Path(paths["json"]).read_text(encoding="utf-8")
    assert "LOC Scanner Context Report" in markdown
    assert "tracer_core/example.cpp" in markdown
    assert "refactoring_guidance.md" in markdown
    assert "Module Baseline Summary" in markdown
    assert '"report_type": "loc_profile_context"' in report_json
