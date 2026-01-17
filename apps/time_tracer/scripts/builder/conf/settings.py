import tomllib
import sys
from pathlib import Path

# 当前文件在 scripts/builder/，配置在 scripts/，所以需要 .parent.parent
# 或者更稳健的方式：传入 base_dir
class AppConfig:
    def __init__(self, config_file_path: Path):
        self._load(config_file_path)

    def _load(self, path: Path):
        if not path.exists():
            print(f"错误: 配置文件 '{path}' 未找到。")
            sys.exit(1)
            
        with open(path, "rb") as f:
            data = tomllib.load(f)
            
        build_settings = data.get('build', {})
        self.COMPILER = build_settings.get('compiler', 'default')
        self.WARNING_LEVEL = build_settings.get('warning_level', 2)
        self.ENABLE_LTO = build_settings.get('enable_lto', True)