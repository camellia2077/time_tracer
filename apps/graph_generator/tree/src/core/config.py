import os
import tomllib
from dataclasses import dataclass, field
from typing import Any, Dict, Mapping, Optional


_ALLOWED_LAYOUTS = {"horizontal", "vertical"}
_ALLOWED_OUTPUT_FORMATS = {"png", "svg", "pdf"}


@dataclass(slots=True)
class TreePaths:
    database: str = ""
    output_directory: str = ""


@dataclass(slots=True)
class TreeSettings:
    root_path: str = ""
    max_depth: int = -1
    layout: str = "horizontal"
    split_roots: bool = False
    output_name: str = "project_tree"
    title: str = "Project Tree"
    output_format: str = "png"
    output_formats: list[str] = field(default_factory=list)
    output_html: bool = False
    output_json: bool = False
    output_images: bool = True


@dataclass(slots=True)
class TreeStyle:
    background_color: str = "#F7F7F7"
    figure_width: float = 16.0
    figure_height: float = 10.0
    dpi: int = 300
    node_size: float = 280.0
    root_color: str = "#4C78A8"
    edge_color: str = "#888888"
    label_font_size: int = 10
    label_offset_y: float = 0.25
    horizontal_spacing: float = 1.4
    vertical_spacing: float = 1.2
    root_node_scale: float = 1.5
    min_node_scale: float = 0.6
    label_root_scale: float = 1.2
    label_min_scale: float = 0.85


@dataclass(slots=True)
class TreeColorScheme:
    root: str
    edge: str


@dataclass(slots=True)
class TreeColors:
    scheme: Optional[str] = None
    schemes: Dict[str, TreeColorScheme] = field(default_factory=dict)


class TreeConfig:
    """Loads, validates, and exposes typed tree generator configuration."""

    def __init__(self, config_path: str):
        if not os.path.exists(config_path):
            print(f"Error: config file not found: {config_path}")
            raise SystemExit(1)

        data = self._load_toml(config_path)
        self.paths = self._parse_paths(data.get("paths", {}))
        self.settings = self._parse_settings(data.get("settings", {}))
        self.style = self._parse_style(data.get("style", {}))
        self.colors = self._parse_colors(data.get("colors", {}))
        self._validate()

    def _load_toml(self, path: str) -> Dict[str, Any]:
        try:
            with open(path, "rb") as handle:
                return tomllib.load(handle)
        except tomllib.TOMLDecodeError as exc:
            print(f"Error: failed to parse TOML: {path}\n{exc}")
            raise SystemExit(1)

    def _parse_paths(self, raw: Mapping[str, Any]) -> TreePaths:
        return TreePaths(
            database=self._to_str(raw.get("database", "")),
            output_directory=self._to_str(raw.get("output_directory", "")),
        )

    def _parse_settings(self, raw: Mapping[str, Any]) -> TreeSettings:
        output_formats_raw = raw.get("output_formats", [])
        return TreeSettings(
            root_path=self._to_str(raw.get("root_path", "")),
            max_depth=self._to_int(raw.get("max_depth", -1), -1),
            layout=self._to_str(raw.get("layout", "horizontal"), "horizontal"),
            split_roots=self._to_bool(raw.get("split_roots", False), False),
            output_name=self._to_str(raw.get("output_name", "project_tree"), "project_tree"),
            title=self._to_str(raw.get("title", "Project Tree"), "Project Tree"),
            output_format=self._to_str(raw.get("output_format", "png"), "png"),
            output_formats=self._to_formats(output_formats_raw),
            output_html=self._to_bool(raw.get("output_html", False), False),
            output_json=self._to_bool(raw.get("output_json", False), False),
            output_images=self._to_bool(raw.get("output_images", True), True),
        )

    def _parse_style(self, raw: Mapping[str, Any]) -> TreeStyle:
        return TreeStyle(
            background_color=self._to_str(raw.get("background_color", "#F7F7F7"), "#F7F7F7"),
            figure_width=self._to_float(raw.get("figure_width", 16.0), 16.0),
            figure_height=self._to_float(raw.get("figure_height", 10.0), 10.0),
            dpi=self._to_int(raw.get("dpi", 300), 300),
            node_size=self._to_float(raw.get("node_size", 280.0), 280.0),
            root_color=self._to_str(raw.get("root_color", "#4C78A8"), "#4C78A8"),
            edge_color=self._to_str(raw.get("edge_color", "#888888"), "#888888"),
            label_font_size=self._to_int(raw.get("label_font_size", 10), 10),
            label_offset_y=self._to_float(raw.get("label_offset_y", 0.25), 0.25),
            horizontal_spacing=self._to_float(raw.get("horizontal_spacing", 1.4), 1.4),
            vertical_spacing=self._to_float(raw.get("vertical_spacing", 1.2), 1.2),
            root_node_scale=self._to_float(raw.get("root_node_scale", 1.5), 1.5),
            min_node_scale=self._to_float(raw.get("min_node_scale", 0.6), 0.6),
            label_root_scale=self._to_float(raw.get("label_root_scale", 1.2), 1.2),
            label_min_scale=self._to_float(raw.get("label_min_scale", 0.85), 0.85),
        )

    def _parse_colors(self, raw: Mapping[str, Any]) -> TreeColors:
        scheme_raw = raw.get("scheme", raw.get("color_scheme", None))
        schemes: Dict[str, TreeColorScheme] = {}
        schemes_raw = raw.get("schemes", {})
        if isinstance(schemes_raw, Mapping):
            for key, value in schemes_raw.items():
                if not isinstance(value, Mapping):
                    continue
                root = self._to_str(value.get("root", ""))
                edge = self._to_str(value.get("edge", ""))
                if root and edge:
                    schemes[str(key)] = TreeColorScheme(root=root, edge=edge)

        scheme_text = self._to_str(scheme_raw, "")
        return TreeColors(scheme=scheme_text or None, schemes=schemes)

    def apply_environment(self, env: Optional[Mapping[str, str]] = None) -> None:
        source = dict(os.environ) if env is None else dict(env)
        path_overrides: Dict[str, Optional[str]] = {}
        setting_overrides: Dict[str, Any] = {}

        database_path = self._first_non_empty(source, "TREE_DB_PATH", "TREE_DATABASE")
        output_directory = self._first_non_empty(source, "TREE_OUTPUT_DIR", "TREE_OUTPUT_DIRECTORY")
        if database_path is not None:
            path_overrides["database"] = database_path
        if output_directory is not None:
            path_overrides["output_directory"] = output_directory

        env_setting_keys = {
            "TREE_ROOT_PATH": "root_path",
            "TREE_MAX_DEPTH": "max_depth",
            "TREE_LAYOUT": "layout",
            "TREE_SPLIT_ROOTS": "split_roots",
            "TREE_OUTPUT_NAME": "output_name",
            "TREE_TITLE": "title",
            "TREE_OUTPUT_FORMAT": "output_format",
            "TREE_OUTPUT_FORMATS": "output_formats",
            "TREE_OUTPUT_HTML": "output_html",
            "TREE_OUTPUT_JSON": "output_json",
            "TREE_OUTPUT_IMAGES": "output_images",
        }
        for env_key, setting_key in env_setting_keys.items():
            if env_key not in source:
                continue
            setting_overrides[setting_key] = source.get(env_key)

        self.apply_overrides(
            paths=path_overrides or None,
            settings=setting_overrides or None,
        )

    def apply_overrides(
        self,
        paths: Optional[Dict[str, Optional[str]]] = None,
        settings: Optional[Dict[str, Any]] = None,
    ) -> None:
        if paths:
            if "database" in paths:
                self.paths.database = self._to_str(paths.get("database", ""))
            if "output_directory" in paths:
                self.paths.output_directory = self._to_str(paths.get("output_directory", ""))

        if settings:
            if "root_path" in settings:
                self.settings.root_path = self._to_str(settings.get("root_path", ""))
            if "max_depth" in settings:
                self.settings.max_depth = self._to_int(settings.get("max_depth", -1), -1)
            if "layout" in settings:
                self.settings.layout = self._to_str(settings.get("layout", "horizontal"), "horizontal")
            if "split_roots" in settings:
                self.settings.split_roots = self._to_bool(settings.get("split_roots", False), False)
            if "output_name" in settings:
                self.settings.output_name = self._to_str(settings.get("output_name", "project_tree"), "project_tree")
            if "title" in settings:
                self.settings.title = self._to_str(settings.get("title", "Project Tree"), "Project Tree")
            if "output_format" in settings:
                self.settings.output_format = self._to_str(settings.get("output_format", "png"), "png")
            if "output_formats" in settings:
                self.settings.output_formats = self._to_formats(settings.get("output_formats", []))
            if "output_html" in settings:
                self.settings.output_html = self._to_bool(settings.get("output_html", False), False)
            if "output_json" in settings:
                self.settings.output_json = self._to_bool(settings.get("output_json", False), False)
            if "output_images" in settings:
                self.settings.output_images = self._to_bool(settings.get("output_images", True), True)

        self._validate()

    def resolved_output_formats(self) -> list[str]:
        if self.settings.output_formats:
            return list(self.settings.output_formats)
        return [self.settings.output_format]

    def _validate(self) -> None:
        self.paths.database = self.paths.database.strip()
        self.paths.output_directory = self.paths.output_directory.strip()

        self.settings.root_path = self.settings.root_path.strip()
        self.settings.output_name = self.settings.output_name.strip() or "project_tree"
        self.settings.title = self.settings.title.strip() or "Project Tree"

        if self.settings.max_depth < -1:
            self.settings.max_depth = -1

        layout = self.settings.layout.strip().lower()
        if layout not in _ALLOWED_LAYOUTS:
            layout = "horizontal"
        self.settings.layout = layout

        output_format = self.settings.output_format.strip().lower()
        if output_format not in _ALLOWED_OUTPUT_FORMATS:
            output_format = "png"
        self.settings.output_format = output_format

        cleaned_formats: list[str] = []
        for output in self.settings.output_formats:
            candidate = str(output).strip().lower()
            if not candidate or candidate not in _ALLOWED_OUTPUT_FORMATS:
                continue
            if candidate not in cleaned_formats:
                cleaned_formats.append(candidate)
        self.settings.output_formats = cleaned_formats

        if self.style.figure_width <= 0:
            self.style.figure_width = 16.0
        if self.style.figure_height <= 0:
            self.style.figure_height = 10.0
        if self.style.dpi <= 0:
            self.style.dpi = 300
        if self.style.node_size <= 0:
            self.style.node_size = 280.0
        if self.style.label_font_size <= 0:
            self.style.label_font_size = 10
        if self.style.horizontal_spacing <= 0:
            self.style.horizontal_spacing = 1.4
        if self.style.vertical_spacing <= 0:
            self.style.vertical_spacing = 1.2
        if self.style.root_node_scale <= 0:
            self.style.root_node_scale = 1.5
        if self.style.min_node_scale <= 0:
            self.style.min_node_scale = 0.6
        if self.style.label_root_scale <= 0:
            self.style.label_root_scale = 1.2
        if self.style.label_min_scale <= 0:
            self.style.label_min_scale = 0.85

    @staticmethod
    def _first_non_empty(source: Mapping[str, str], *keys: str) -> Optional[str]:
        for key in keys:
            value = source.get(key)
            if value is None:
                continue
            text = str(value).strip()
            if text:
                return text
        return None

    @staticmethod
    def _to_str(value: Any, default: str = "") -> str:
        if value is None:
            return default
        return str(value)

    @staticmethod
    def _to_int(value: Any, default: int) -> int:
        try:
            return int(value)
        except (TypeError, ValueError):
            return default

    @staticmethod
    def _to_float(value: Any, default: float) -> float:
        try:
            return float(value)
        except (TypeError, ValueError):
            return default

    @staticmethod
    def _to_bool(value: Any, default: bool) -> bool:
        if isinstance(value, bool):
            return value
        if value is None:
            return default
        text = str(value).strip().lower()
        if text in {"1", "true", "yes", "on"}:
            return True
        if text in {"0", "false", "no", "off"}:
            return False
        return default

    @staticmethod
    def _to_formats(value: Any) -> list[str]:
        if isinstance(value, list):
            candidates = [str(item) for item in value]
        elif isinstance(value, str):
            candidates = value.split(",")
        elif value is None:
            candidates = []
        else:
            candidates = [str(value)]

        formats: list[str] = []
        for candidate in candidates:
            normalized = candidate.strip().lower()
            if normalized:
                formats.append(normalized)
        return formats
