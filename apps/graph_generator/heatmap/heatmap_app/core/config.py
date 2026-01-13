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
        self.bool_heatmap_settings = main_config.get("boolean_heatmaps", {})

        if not str(self.db_path):
            print("错误：数据库路径未在配置中指定。")
            sys.exit(1)

        # --- [新增] 检查数据库文件是否存在 ---
        if not self.db_path.exists():
            # 如果是相对路径，为了清晰可以打印绝对路径
            print(f"错误：无法找到数据库文件: {self.db_path.resolve()}") 
            sys.exit(1)