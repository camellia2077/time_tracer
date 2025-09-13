import tomllib
import os
from typing import Dict, Any

class TimelineConfig:
    """加载并持有时间线图表的所有应用配置。"""
    def __init__(self, config_path: str):
        if not os.path.exists(config_path):
            print(f"错误: 配置文件 '{config_path}' 未找到。")
            exit(1)
            
        self.config_data = self._load_toml(config_path)
        
    def _load_toml(self, path: str) -> Dict[str, Any]:
        """从 TOML 文件加载配置。"""
        try:
            with open(path, "rb") as f:
                return tomllib.load(f)
        except tomllib.TOMLDecodeError as e:
            print(f"错误: 解析配置文件 '{path}' 失败: {e}")
            exit(1)

    def get_path(self, key: str) -> str:
        return self.config_data.get("paths", {}).get(key, "")

    def get_setting(self, key: str) -> str:
        return self.config_data.get("settings", {}).get(key, "")

    def get_colors(self) -> Dict[str, str]:
        return self.config_data.get("colors", {})