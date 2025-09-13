# graph_generator/core/config.py
import sys
import tomllib
from typing import Any, Dict

# 将颜色常量也集中到配置模块中
COLOR_GREEN = '\033[92m'
COLOR_RED = '\033[91m'
COLOR_RESET = '\033[0m'
COLOR_YELLOW = '\033[93m'

def load_toml_config(path: str) -> Dict[str, Any]:
    """
    一个通用的TOML配置加载函数，包含详细的错误处理。
    """
    try:
        with open(path, 'rb') as f:
            return tomllib.load(f)
    except FileNotFoundError:
        print(f"{COLOR_RED}错误: 配置文件未找到 '{path}'。{COLOR_RESET}", file=sys.stderr)
        sys.exit(1)
    except tomllib.TOMLDecodeError as e:
        print(f"{COLOR_RED}错误: 无法解析TOML文件 '{path}': {e}。{COLOR_RESET}", file=sys.stderr)
        sys.exit(1)

class AppConfig:
    """
    一个单例类，用于加载和提供所有应用程序配置。
    """
    _instance = None

    def __new__(cls):
        if cls._instance is None:
            cls._instance = super(AppConfig, cls).__new__(cls)
            cls._instance._load_configs()
        return cls._instance

    def _load_configs(self):
        print(f"{COLOR_YELLOW}正在加载所有配置文件...{COLOR_RESET}")
        self.timeline_config = load_toml_config('configs/timeline_colors.toml')
        self.heatmap_config = load_toml_config('configs/heatmap_colors.toml')
        print(f"{COLOR_GREEN}✅ 所有配置加载成功。{COLOR_RESET}")

    def get_timeline_color_map(self) -> Dict[str, str]:
        active_scheme = self.timeline_config.get('active_scheme', 'default')
        return self.timeline_config.get('color_schemes', {}).get(active_scheme, {})

    def get_heatmap_config(self) -> Dict[str, Any]:
        return self.heatmap_config

# 创建一个全局可访问的配置实例
app_config = AppConfig()