import tomllib
from pathlib import Path
from colors import AnsiColors

# 获取当前脚本文件所在的目录
SCRIPT_DIR = Path(__file__).resolve().parent

class AppConfig:
    """加载并持有所有构建配置。"""
    def __init__(self, config_path="config.toml"):
        full_config_path = SCRIPT_DIR / config_path
        config_data = self._load_config(full_config_path)
        
        # --- 从 TOML 加载构建设置 ---
        build_settings = config_data.get('build', {})
        self.COMPILER = build_settings.get('compiler', 'default')
        self.WARNING_LEVEL = build_settings.get('warning_level', 2)
        # ==================== [核心修改] ====================
        # 加载LTO设置，如果未在文件中指定，则默认为True
        self.ENABLE_LTO = build_settings.get('enable_lto', True)
        # ====================================================

        # --- 直接从导入的类中赋值颜色 ---
        self.HEADER = AnsiColors.HEADER
        self.OKBLUE = AnsiColors.OKBLUE
        self.OKGREEN = AnsiColors.OKGREEN
        self.WARNING = AnsiColors.WARNING
        self.FAIL = AnsiColors.FAIL
        self.ENDC = AnsiColors.ENDC
        self.BOLD = AnsiColors.BOLD

    def _load_config(self, path):
        """从 TOML 文件加载配置。"""
        try:
            with open(path, "rb") as f:
                return tomllib.load(f)
        except FileNotFoundError:
            print(f"错误: 配置文件 '{path}' 未找到。")
            exit(1)

# 创建一个全局实例，供其他模块导入和使用
config = AppConfig()