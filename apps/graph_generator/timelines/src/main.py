import os
# ✨ 核心修改：将相对导入改为从 src 根目录开始的绝对导入
from core.config import TimelineConfig
from services.timeline_service import TimelineService

def run_timeline_generator(base_dir: str):
    """
    初始化并运行时间线图表生成器。
    
    Args:
        base_dir (str): 项目的根目录路径。
    """
    print("--- 初始化时间线图表生成器 ---")
    config_path = os.path.join(base_dir, "configs", "timeline_config.toml")
    
    # 1. 加载配置
    config = TimelineConfig(config_path)
    
    # 2. 初始化并运行核心服务
    service = TimelineService(config)
    service.generate_timeline()