import tomllib
import os
from typing import Dict, Any

class AppConfig:
    """加载并持有所有应用配置的单例类。"""
    def __init__(self, config_dir: str):
        self._config_path = os.path.join(config_dir, "heatmap_config.toml")
        self._colors_path = os.path.join(config_dir, "heatmap_colors.toml")
        
        self.heatmap_settings: Dict[str, Any] = {}
        self.db_path: str = ""
        self.bool_heatmap_settings: Dict[str, Any] = {}
        self.color_settings: Dict[str, Any] = {}
        
        self._load_configs()

    def _load_toml(self, path: str) -> Dict[str, Any]:
        """从 TOML 文件加载配置。"""
        try:
            with open(path, "rb") as f:
                return tomllib.load(f)
        except FileNotFoundError:
            print(f"错误: 配置文件 '{path}' 未找到。")
            exit(1)

    def _load_configs(self):
        """加载并解析所有配置文件。"""
        main_config = self._load_toml(self._config_path)
        self.color_settings = self._load_toml(self._colors_path)
        
        self.db_path = main_config.get("database", {}).get("path", "")
        self.heatmap_settings = main_config.get("heatmap", {})
        self.bool_heatmap_settings = main_config.get("boolean_heatmaps", {})

        if not self.db_path:
            print("错误：数据库路径未在配置中指定。")
            exit(1)