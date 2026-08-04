from dataclasses import dataclass
from pathlib import Path

try:
    import tomllib
except ModuleNotFoundError:  # pragma: no cover
    import tomli as tomllib  # type: ignore

LANG_CHOICES = ("cpp", "kt", "py", "rs")
PATH_MODE_CHOICES = ("cli_override", "toml_only", "merge")
DEFAULT_TEST_DIRECTORY_NAMES = ("test", "tests", "androidtest", "testfixtures")


@dataclass(frozen=True)
class LanguageConfig:
    lang: str
    display_name: str
    default_paths: list[str]
    extensions: set[str]
    ignore_dirs: set[str]
    ignore_prefixes: tuple[str, ...]
    path_mode: str
    default_over_threshold: int
    default_under_threshold: int
    default_dir_over_files: int
    over_inclusive: bool
    test_directory_names: tuple[str, ...]


@dataclass(frozen=True)
class ComponentConfig:
    name: str
    display_name: str
    root: str
    category: str
    priority: int
    exclude_roots: tuple[str, ...] = ()
    test_roots: tuple[str, ...] = ()


@dataclass(frozen=True)
class ScanProfile:
    name: str
    display_name: str
    components: tuple[ComponentConfig, ...]
    test_directory_names: tuple[str, ...]
    test_priority: int
    languages: tuple[str, ...] = LANG_CHOICES


def load_language_config(config_path: Path, lang: str) -> LanguageConfig:
    payload = _load_toml_payload(config_path)

    section = payload.get(lang)
    if not isinstance(section, dict):
        raise ValueError(f"配置缺失语言节点: [{lang}]")

    display_name = str(section.get("display_name", lang)).strip() or lang
    default_paths = _as_str_list(section.get("default_paths"), f"{lang}.default_paths")
    extensions = {
        item.lower() for item in _as_str_list(section.get("extensions"), f"{lang}.extensions")
    }
    ignore_dirs = set(_as_str_list(section.get("ignore_dirs"), f"{lang}.ignore_dirs"))
    ignore_prefixes = tuple(
        item.lower()
        for item in _as_str_list(section.get("ignore_prefixes"), f"{lang}.ignore_prefixes")
    )
    path_mode = _as_path_mode(section.get("path_mode", "cli_override"), f"{lang}.path_mode")
    default_over_threshold = _as_positive_int(
        section.get("default_over_threshold"),
        f"{lang}.default_over_threshold",
    )
    default_under_threshold = _as_positive_int(
        section.get("default_under_threshold"),
        f"{lang}.default_under_threshold",
    )
    default_dir_over_files = _as_positive_int(
        section.get("default_dir_over_files"),
        f"{lang}.default_dir_over_files",
    )
    over_inclusive = bool(section.get("over_inclusive", False))
    test_directory_names, _ = _load_test_classification(payload)

    return LanguageConfig(
        lang=lang,
        display_name=display_name,
        default_paths=default_paths,
        extensions=extensions,
        ignore_dirs=ignore_dirs,
        ignore_prefixes=ignore_prefixes,
        path_mode=path_mode,
        default_over_threshold=default_over_threshold,
        default_under_threshold=default_under_threshold,
        default_dir_over_files=default_dir_over_files,
        over_inclusive=over_inclusive,
        test_directory_names=test_directory_names,
    )


def load_scan_profile(config_path: Path, profile_name: str) -> ScanProfile:
    payload = _load_toml_payload(config_path)
    profiles = payload.get("profiles")
    if not isinstance(profiles, dict):
        raise ValueError("配置缺失 [profiles.*] 节点")

    section = profiles.get(profile_name)
    if not isinstance(section, dict):
        raise ValueError(f"配置缺失扫描 profile: [profiles.{profile_name}]")

    component_names = _as_str_list(
        section.get("components"),
        f"profiles.{profile_name}.components",
    )
    components_payload = payload.get("components")
    if not isinstance(components_payload, dict):
        raise ValueError("配置缺失 [components.*] 节点")

    components: list[ComponentConfig] = []
    for component_name in component_names:
        component_payload = components_payload.get(component_name)
        if not isinstance(component_payload, dict):
            raise ValueError(
                f"配置缺失组件: [components.{component_name}]"
            )
        root = _as_non_empty_string(
            component_payload.get("root"),
            f"components.{component_name}.root",
        )
        category = _as_non_empty_string(
            component_payload.get("category"),
            f"components.{component_name}.category",
        )
        priority = _as_non_negative_int(
            component_payload.get("priority"),
            f"components.{component_name}.priority",
        )
        exclude_roots = tuple(
            _as_str_list(
                component_payload.get("exclude_roots", []),
                f"components.{component_name}.exclude_roots",
            )
        )
        test_roots = tuple(
            _as_str_list(
                component_payload.get("test_roots", []),
                f"components.{component_name}.test_roots",
            )
        )
        display_name = str(
            component_payload.get("display_name", component_name)
        ).strip() or component_name
        components.append(
            ComponentConfig(
                name=component_name,
                display_name=display_name,
                root=root,
                category=category,
                priority=priority,
                exclude_roots=exclude_roots,
                test_roots=test_roots,
            )
        )

    test_directory_names, test_priority = _load_test_classification(payload)
    raw_languages = section.get("languages", list(LANG_CHOICES))
    languages = tuple(_as_str_list(raw_languages, f"profiles.{profile_name}.languages"))
    unknown_languages = sorted(set(languages) - set(LANG_CHOICES))
    if unknown_languages:
        raise ValueError(
            f"profile languages must be from {LANG_CHOICES}: {unknown_languages}"
        )
    if not languages:
        raise ValueError(f"profile languages cannot be empty: profiles.{profile_name}.languages")

    display_name = str(section.get("display_name", profile_name)).strip()
    return ScanProfile(
        name=profile_name,
        display_name=display_name or profile_name,
        components=tuple(components),
        test_directory_names=test_directory_names,
        test_priority=test_priority,
        languages=languages,
    )


def _load_test_classification(payload: dict) -> tuple[tuple[str, ...], int]:
    test_classification = payload.get("test_classification", {})
    if not isinstance(test_classification, dict):
        raise ValueError("配置字段必须是表: test_classification")

    raw_test_directory_names = test_classification.get(
        "directory_names",
        list(DEFAULT_TEST_DIRECTORY_NAMES),
    )
    test_directory_names = tuple(
        item.lower()
        for item in _as_str_list(
            raw_test_directory_names,
            "test_classification.directory_names",
        )
    )
    test_priority = _as_non_negative_int(
        test_classification.get("priority", 3),
        "test_classification.priority",
    )
    return test_directory_names, test_priority


def _load_toml_payload(config_path: Path) -> dict:
    if not config_path.exists():
        raise FileNotFoundError(f"配置文件不存在: {config_path}")

    with config_path.open("rb") as handle:
        return tomllib.load(handle)


def _as_str_list(value, field_name: str) -> list[str]:
    if not isinstance(value, list):
        raise ValueError(f"配置字段必须是数组: {field_name}")
    output: list[str] = []
    for item in value:
        if not isinstance(item, str) or not item.strip():
            raise ValueError(f"配置字段元素必须是非空字符串: {field_name}")
        output.append(item.strip())
    return output


def _as_non_empty_string(value, field_name: str) -> str:
    if not isinstance(value, str) or not value.strip():
        raise ValueError(f"配置字段必须是非空字符串: {field_name}")
    return value.strip()


def _as_positive_int(value, field_name: str) -> int:
    if not isinstance(value, int) or value <= 0:
        raise ValueError(f"配置字段必须是正整数: {field_name}")
    return value


def _as_non_negative_int(value, field_name: str) -> int:
    if not isinstance(value, int) or value < 0:
        raise ValueError(f"配置字段必须是非负整数: {field_name}")
    return value


def _as_path_mode(value, field_name: str) -> str:
    if not isinstance(value, str):
        raise ValueError(f"配置字段必须是字符串: {field_name}")

    normalized = value.strip().lower()
    if normalized not in PATH_MODE_CHOICES:
        raise ValueError(
            f"配置字段必须是 {PATH_MODE_CHOICES} 之一: {field_name}"
        )
    return normalized
