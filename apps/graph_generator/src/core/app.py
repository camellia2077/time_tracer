# graph_generator/core/app.py
from datetime import datetime
from data import db_access, day_analyzer
from rendering import plotters, heatmap_renderer
from strategies import heatmap_strategies
from core.config import app_config, COLOR_GREEN, COLOR_RED, COLOR_YELLOW

class App:
    """
    应用程序的核心逻辑和流程编排器。
    """
    def generate_day_chart(self, date_str: str, chart_type: str):
        print(f"🚀 开始为 {date_str} 生成 {chart_type} 图表...")
        color_map = app_config.get_timeline_color_map()
        
        print(f"{COLOR_YELLOW}正在处理数据...{COLOR_RESET}")
        processor = day_analyzer.DataProcessor()
        logical_day = processor.create_logical_day(date_str)
        
        if not logical_day or logical_day.processed_data.empty:
            print(f"{COLOR_RED}❌ 未能找到或处理该日期的数据。{COLOR_RESET}")
            return

        print(f"{COLOR_GREEN}✅ 数据处理成功。{COLOR_RESET}")
        formatted_date = datetime.strptime(date_str, "%Y%m%d").strftime('%B %d, %Y')
        output_filename = f"{chart_type}_{date_str}.png"
        
        if chart_type == 'timeline':
            plotter = plotters.TimelinePlotter(logical_day, color_map)
            title = f"每日活动时间线 - {formatted_date}"
            plotter.save_chart(output_filename, title)
        elif chart_type == 'barchart':
            plotter = plotters.BarChartPlotter(logical_day, color_map)
            title = f"各项活动总时长分析 ({formatted_date})"
            plotter.save_chart(output_filename, title)
            
        print(f"\n🎉 {COLOR_GREEN}成功生成图表: {output_filename}{COLOR_RESET}")

    def generate_heatmap(self, year: int, heatmap_type: str, project: str = "mystudy"):
        print(f"🚀 开始为 {year} 年生成 {heatmap_type} 热力图...")
        strategy = None
        data = None
        
        if heatmap_type == 'project':
            data = db_access.get_data_for_heatmap(year, project)
            if not data:
                print(f"{COLOR_RED}❌ 未找到项目 '{project}' 的数据。{COLOR_RESET}")
                return
            strategy = heatmap_strategies.NumericStrategy(project)
            base_filename = f"heatmap_{project}_{year}"
        elif heatmap_type == 'sleep':
            data = db_access.get_sleep_data_for_bool_heatmap(year)
            if data is None:
                print(f"{COLOR_RED}❌ 无法获取睡眠数据。{COLOR_RESET}")
                return
            strategy = heatmap_strategies.BooleanStrategy()
            base_filename = f"{year}_sleep_heatmap"

        renderer = heatmap_renderer.HeatmapRenderer(year, data, strategy)
        
        annual_filename = f"{base_filename}_annual.html"
        monthly_filename = f"{base_filename}_monthly.html"

        renderer.save_annual_heatmap(annual_filename)
        renderer.save_monthly_heatmap(monthly_filename)

        print(f"\n🎉 {COLOR_GREEN}成功生成热力图:{COLOR_RESET}")
        print(f"  - 年度: {annual_filename}")
        print(f"  - 月度: {monthly_filename}")