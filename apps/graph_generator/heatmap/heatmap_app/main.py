from .core.config import AppConfig
from .data.sqlite_source import SQLiteSource
from .services.generator_service import GeneratorService

def run_generator(base_dir: str):
    """
    初始化并运行热力图生成器。
    
    Args:
        base_dir (str): 项目的根目录路径。
    """
    print("--- 初始化热力图生成器 ---")
    config_dir = f"{base_dir}/configs"
    
    # 1. 加载配置
    config = AppConfig(config_dir)
    
    # 2. 初始化数据源
    data_source = SQLiteSource(config.db_path)
    
    # 3. 初始化并运行核心服务
    service = GeneratorService(config, data_source, base_dir)
    service.generate_all()