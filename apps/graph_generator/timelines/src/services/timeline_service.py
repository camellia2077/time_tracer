from core.config import TimelineConfig
from data.sqlite_source import TimelineSQLiteSource
from rendering.chart_renderer import ChartRenderer

class TimelineService:
    """核心服务，负责编排时间线图表的生成流程。"""
    
    # ✨ 核心修改：增加 default_output_dir 参数
    def __init__(self, config: TimelineConfig, default_output_dir: str = ""):
        self.config = config
        
        self.data_source = TimelineSQLiteSource(config.get_path("database"))
        
        # ✨ 核心修改：获取输出路径时传入默认值
        # 如果 toml 中 output_directory 为空，就会使用 default_output_dir
        final_output_dir = config.get_path("output_directory", default=default_output_dir)
        
        print(f"输出目录: {final_output_dir}") # 打印一下确认路径
        self.renderer = ChartRenderer(final_output_dir)

    def generate_timeline(self):
        """生成时间线图表。"""
        target_date = self.config.get_setting("target_date")
        if not target_date:
            print("错误：配置文件中未指定 target_date。")
            return
            
        # 1. 获取数据
        records = self.data_source.fetch_records_for_date(target_date)
        if not records:
            return

        # 2. 获取颜色配置
        colors = self.config.get_colors()

        # 3. 渲染图表
        self.renderer.render(records, colors, target_date)
        print("\n时间线图表生成完毕！")