import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC_ROOT = PROJECT_ROOT / "src"
if str(SRC_ROOT) not in sys.path:
    sys.path.insert(0, str(SRC_ROOT))

from loc_scanner.config import load_language_config, load_scan_profile


def test_load_default_config_for_python() -> None:
    config_path = PROJECT_ROOT / "config" / "scan_lines.toml"
    config = load_language_config(config_path=config_path, lang="py")

    assert config.lang == "py"
    assert ".py" in config.extensions
    assert config.default_over_threshold == 200
    assert config.default_under_threshold > 0
    assert config.path_mode == "cli_override"


def test_load_default_config_for_cpp_uses_toml_only_paths() -> None:
    config_path = PROJECT_ROOT / "config" / "scan_lines.toml"
    config = load_language_config(config_path=config_path, lang="cpp")

    assert config.path_mode == "toml_only"
    assert config.default_paths == ["apps", "libs"]


def test_load_core_family_profile() -> None:
    config_path = PROJECT_ROOT / "config" / "scan_lines.toml"
    profile = load_scan_profile(config_path=config_path, profile_name="core_family")

    assert profile.display_name == "Core Family"
    assert [component.name for component in profile.components] == [
        "tracer_core",
        "tracer_adapters_io",
        "tracer_core_bridge_common",
        "tracer_transport",
    ]
    assert profile.components[0].category == "libs"
    assert profile.components[0].priority == 0
    assert profile.test_priority == 3
    assert "tests" in profile.test_directory_names
    assert "androidtest" in profile.test_directory_names


def test_load_android_profile_only_scans_android_component() -> None:
    config_path = PROJECT_ROOT / "config" / "scan_lines.toml"
    profile = load_scan_profile(config_path=config_path, profile_name="android")

    assert profile.display_name == "Android"
    assert [component.name for component in profile.components] == ["android"]
    assert profile.components[0].root == "apps/android"


def test_load_windows_cli_profile_only_scans_rust_cli() -> None:
    config_path = PROJECT_ROOT / "config" / "scan_lines.toml"
    profile = load_scan_profile(config_path=config_path, profile_name="windows_cli")

    assert profile.display_name == "Windows CLI"
    assert [component.name for component in profile.components] == ["windows_cli"]
    assert profile.components[0].root == "apps/cli"
    assert profile.languages == ("rs",)


def test_load_loc_scanner_profile_only_scans_python_tool() -> None:
    config_path = PROJECT_ROOT / "config" / "scan_lines.toml"
    profile = load_scan_profile(config_path=config_path, profile_name="loc_scanner")

    assert profile.display_name == "LOC Scanner"
    assert [component.name for component in profile.components] == ["loc_scanner"]
    assert profile.components[0].root == "tools/devtools/loc_scanner"
    assert profile.components[0].category == "tools"
    assert profile.languages == ("py",)


def test_load_python_tooling_profile_separates_tidy_and_toolchain_glue() -> None:
    config_path = PROJECT_ROOT / "config" / "scan_lines.toml"
    profile = load_scan_profile(config_path=config_path, profile_name="python_tooling")

    assert profile.languages == ("py",)
    assert [component.name for component in profile.components] == [
        "shared_tests",
        "tool_tests",
        "tidy_workflow",
        "clang_adapters",
        "toolchain_glue",
        "loc_scanner",
    ]
    glue = profile.components[4]
    assert glue.exclude_roots == (
        "tools/toolchain/commands/tidy",
        "tools/toolchain/commands/clang",
    )


def test_load_tidy_profile_only_scans_python_clang_tooling() -> None:
    config_path = PROJECT_ROOT / "config" / "scan_lines.toml"
    profile = load_scan_profile(config_path=config_path, profile_name="tidy")

    assert profile.languages == ("py",)
    assert [component.name for component in profile.components] == [
        "tidy_workflow",
        "clang_adapters",
    ]
    assert profile.components[0].test_roots == ("tools/tests/platform/tidy",)


def test_load_workspace_profile_includes_standalone_test_components() -> None:
    config_path = PROJECT_ROOT / "config" / "scan_lines.toml"
    profile = load_scan_profile(config_path=config_path, profile_name="workspace")

    component_names = [component.name for component in profile.components]
    assert component_names[-2:] == ["shared_tests", "tool_tests"]
    assert all(
        component.category == "tests"
        and component.priority == 3
        for component in profile.components[-2:]
    )
