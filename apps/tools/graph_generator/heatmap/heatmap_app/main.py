from pathlib import Path
from .core.config import AppConfig
from .data.core_contract_source import CoreContractSource
from .data.sqlite_source import SQLiteSource
from .services.generator_service import GeneratorService

def run_generator(base_dir_str: str):
    """
    初始化并运行热力图生成器。
    Args:
        base_dir_str (str): 项目的根目录路径。
    """
    base_dir = Path(base_dir_str)
    print("--- 初始化热力图生成器 ---")
    config_dir = base_dir / "configs"
    
    # 1. 加载配置
    config = AppConfig(config_dir)
    
    # 2. 初始化数据源（逐步迁移：支持 Core 契约 + SQL 回退）
    sqlite_source = SQLiteSource(config.db_path)
    if config.source_mode == "core_contract":
        print("数据源模式: core_contract（数值热力图走 Core 契约）")
        data_source = CoreContractSource(
            db_path=config.db_path,
            cli_path=config.core_cli_path,
            timeout_seconds=config.core_timeout_seconds,
            fallback_sqlite=sqlite_source,
            allow_sql_fallback=config.allow_sql_fallback,
        )
    else:
        print("数据源模式: sqlite（直连 SQL）")
        data_source = sqlite_source

    # 3. 初始化并运行核心服务
    service = GeneratorService(config, data_source, base_dir)
    service.generate_all()
