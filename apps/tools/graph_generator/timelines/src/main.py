import os
from core.config import TimelineConfig
from services.timeline_service import TimelineService
from rendering.chart_renderer import ChartRenderer # 需要引入 Renderer 类以便手动注入路径，或者修改 Service 构造函数

# 这里的修改稍微有点棘手，因为 TimelineService 目前是在内部自己 new TimelineSQLiteSource 和 ChartRenderer 的。
# 更好的做法是在 main.py 里解析好路径，然后通过 Config 对象或者直接传参给 Service。

# 我们稍微调整一下 Service 的逻辑，让它支持从 Config 对象中获取处理过的路径。

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
    
    # ✨ 核心修改：定义默认输出目录
    default_output_dir = os.path.join(base_dir, "output")
    
    # 2. 初始化并运行核心服务
    # 我们将默认路径作为上下文传递给 Service，或者更优雅地，我们修改 Config 类让它在内部处理好
    # 但由于 Config 类无法知道 base_dir，我们在这里设置一个临时属性或者直接修改 Service。
    
    # 为了最小化改动，我们直接在 Service 初始化时，通过 Config 获取带默认值的路径
    service = TimelineService(config, default_output_dir) 
    service.generate_timeline()