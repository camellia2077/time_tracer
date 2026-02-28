import tomllib
from pathlib import Path
from typing import Dict, Any
import sys

class AppConfig:
    """加载并持有所有应用配置的单例类。"""
    def __init__(self, config_dir: Path):
        # 确保传入的是 Path 对象
        self.config_dir = Path(config_dir)
        self._config_path = self.config_dir / "heatmap_config.toml"
        self._colors_path = self.config_dir / "heatmap_colors.toml"
        
        self.heatmap_settings: Dict[str, Any] = {}
        self.db_path: Path = Path("")
        self.bool_heatmap_settings: Dict[str, Any] = {}
        self.color_settings: Dict[str, Any] = {}
        self.source_mode: str = "sqlite"
        self.core_cli_path: str = "time_tracer_cli"
        self.core_timeout_seconds: int = 20
        self.allow_sql_fallback: bool = True
        
        self._load_configs()

    def _load_toml(self, path: Path) -> Dict[str, Any]:
        """从 TOML 文件加载配置。"""
        try:
            with path.open("rb") as f:
                return tomllib.load(f)
        except FileNotFoundError:
            print(f"错误: 配置文件 '{path}' 未找到。")
            sys.exit(1)

    def _load_configs(self):
        """加载并解析所有配置文件。"""
        main_config = self._load_toml(self._config_path)
        self.color_settings = self._load_toml(self._colors_path)
        
        db_path_str = main_config.get("database", {}).get("path", "")
        self.db_path = Path(db_path_str)
        
        self.heatmap_settings = main_config.get("heatmap", {})
        # 建议添加一个兼容性处理
        p_names = self.heatmap_settings.get("project_names")
        if p_names is None:
            # 如果没有列表，尝试取单个 project_name 并包装成列表
            single_name = self.heatmap_settings.get("project_name")
            self.heatmap_settings["project_names"] = [single_name] if single_name else []
        else:
            self.heatmap_settings["project_names"] = p_names
        self.bool_heatmap_settings = main_config.get("boolean_heatmaps", {})

        source_cfg = main_config.get("source", {})
        requested_mode = str(source_cfg.get("mode", "sqlite")).strip().lower()
        if requested_mode not in {"sqlite", "core_contract"}:
            print(f"警告: 未知 source.mode='{requested_mode}'，已回退为 sqlite。")
            requested_mode = "sqlite"
        self.source_mode = requested_mode

        self.core_cli_path = (
            str(source_cfg.get("core_cli_path", "time_tracer_cli")).strip()
            or "time_tracer_cli"
        )
        try:
            self.core_timeout_seconds = max(
                1, int(source_cfg.get("core_timeout_seconds", 20))
            )
        except (TypeError, ValueError):
            self.core_timeout_seconds = 20

        fallback_raw = source_cfg.get("allow_sql_fallback", True)
        if isinstance(fallback_raw, bool):
            self.allow_sql_fallback = fallback_raw
        else:
            self.allow_sql_fallback = (
                str(fallback_raw).strip().lower() not in {"0", "false", "no", "off"}
            )

        if not str(self.db_path):
            print("错误：数据库路径未在配置中指定。")
            sys.exit(1)

        # --- [新增] 检查数据库文件是否存在 ---
        if not self.db_path.exists():
            print(f"错误：无法找到数据库文件: {self.db_path}")
            # 如果是相对路径，为了清晰可以打印绝对路径
            print(f"错误：无法找到数据库文件: {self.db_path.resolve()}") 
            sys.exit(1)
