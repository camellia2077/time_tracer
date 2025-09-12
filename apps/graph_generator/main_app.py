import sys
from datetime import datetime

# 从模块中导入核心功能
# 确保 modules/ 文件夹与此脚本在同一目录下
try:
    from modules.day_analyzer import DataProcessor
    from modules.plotters import TimelinePlotter, BarChartPlotter
    from modules.heatmap_generator import HeatmapGenerator, create_numeric_heatmap_strategy, create_boolean_heatmap_strategy
    import db_access
    from db_access import COLOR_RED, COLOR_RESET, COLOR_GREEN
except ImportError as e:
    print(f"错误: 无法导入必要的模块: {e}")
    print("请确保此脚本与 'modules' 文件夹在同一个目录下，并且所有依赖都已安装。")
    sys.exit(1)


def load_json_config(path):
    """一个通用的JSON配置加载函数。"""
    import json
    try:
        with open(path, 'r', encoding='utf-8') as f:
            return json.load(f)
    except FileNotFoundError:
        print(f"{COLOR_RED}错误: 配置文件未找到 '{path}'。{COLOR_RESET}", file=sys.stderr)
        sys.exit(1)
    except json.JSONDecodeError:
        print(f"{COLOR_RED}错误: 无法解析JSON文件 '{path}'。{COLOR_RESET}", file=sys.stderr)
        sys.exit(1)


def run_day_analysis_chart(date_str, chart_type):
    """运行基于“逻辑日”的图表生成（时间线或柱状图）。"""
    print(f"🚀 开始为 {date_str} 生成 {chart_type} 图表...")
    
    colors_path = 'configs/timeline_colors.json'
    color_config = load_json_config(colors_path)
    active_scheme_name = color_config.get('active_scheme', 'default')
    color_map = color_config.get('color_schemes', {}).get(active_scheme_name, {})
    
    try:
        processor = DataProcessor()
        logical_day = processor.create_logical_day(date_str)
    except ConnectionError as e:
        print(f"{COLOR_RED}数据库连接失败: {e}{COLOR_RESET}", file=sys.stderr)
        return

    if not logical_day or logical_day.processed_data.empty:
        print("未能找到或处理该日期的数据，无法生成图表。")
        return

    formatted_date = datetime.strptime(date_str, "%Y%m%d").strftime('%B %d, %Y')
    
    if chart_type == 'timeline':
        plotter = TimelinePlotter(logical_day, color_map)
        output_filename = f"timeline_{date_str}_{active_scheme_name}.png"
        title = f"每日活动时间线 - {formatted_date} (色彩方案: {active_scheme_name})"
        plotter.save_chart(output_filename, title)
    elif chart_type == 'barchart':
        plotter = BarChartPlotter(logical_day, color_map)
        output_filename = f"barchart_{date_str}_{active_scheme_name}.png"
        title = f"各项活动总时长分析 ({formatted_date})"
        plotter.save_chart(output_filename, title)
        
    print(f"\n🎉 {COLOR_GREEN}成功生成图表: {output_filename}{COLOR_RESET}")


def run_heatmap(year, heatmap_type, project=None):
    """运行热力图生成（项目时长或睡眠状态），默认生成两种格式。"""
    print(f"🚀 开始为 {year} 年生成 {heatmap_type} 热力图 (年度和月度)...")
    
    base_filename = ""
    strategy = None

    if heatmap_type == 'project':
        data = db_access.get_data_for_heatmap(year, project)
        strategy = create_numeric_heatmap_strategy('configs/heatmap_colors.json', project)
        base_filename = f"heatmap_{project}_{year}"

    elif heatmap_type == 'sleep':
        data = db_access.get_sleep_data_for_bool_heatmap(year)
        strategy = create_boolean_heatmap_strategy()
        base_filename = f"{year}_sleep_heatmap"
    
    if data is None:
        print(f"{COLOR_RED}无法获取 {year} 年的数据。{COLOR_RESET}", file=sys.stderr)
        return
        
    generator = HeatmapGenerator(year, data, strategy)
    
    annual_filename = f"{base_filename}_annual.html"
    monthly_filename = f"{base_filename}_monthly.html"

    generator.save_annual_heatmap(annual_filename)
    generator.save_monthly_heatmap(monthly_filename)
    
    print(f"\n🎉 {COLOR_GREEN}成功生成两种热力图:{COLOR_RESET}")
    print(f"  - 年度视图: {annual_filename}")
    print(f"  - 月度视图: {monthly_filename}")


def _prompt_for_date():
    """提示用户输入并验证日期。"""
    while True:
        date_str = input("请输入目标日期 (格式 YYYYMMDD): ").strip()
        try:
            datetime.strptime(date_str, "%Y%m%d")
            return date_str
        except ValueError:
            print(f"{COLOR_RED}日期格式错误，请确保格式为 YYYYMMDD (例如: 20240101)。{COLOR_RESET}")

def _prompt_for_year():
    """提示用户输入并验证年份。"""
    while True:
        year_str = input("请输入目标年份 (例如 2024): ").strip()
        if year_str.isdigit() and len(year_str) == 4:
            return int(year_str)
        else:
            print(f"{COLOR_RED}年份格式错误，请输入一个四位数字的年份。{COLOR_RESET}")

def _prompt_for_project():
    """提示用户输入项目名称，支持默认值。"""
    project = input("请输入目标父项目名称 [默认为: mystudy]: ").strip()
    return project.lower() if project else "mystudy"


def main():
    """程序主入口：运行交互式菜单。"""
    print("="*50)
    print("欢迎使用可视化工具交互模式！")
    print("="*50)

    while True:
        print("\n请选择要生成的可视化类型:")
        print("  1. 时间线图 (Timeline)")
        print("  2. 柱状图 (Bar Chart)")
        print("  3. 项目热力图 (Project Heatmap)")
        print("  4. 睡眠热力图 (Sleep Heatmap)")
        print("  q. 退出 (Quit)")
        
        choice = input("> ").strip()

        if choice == '1':
            date = _prompt_for_date()
            run_day_analysis_chart(date, 'timeline')
        elif choice == '2':
            date = _prompt_for_date()
            run_day_analysis_chart(date, 'barchart')
        elif choice == '3':
            year = _prompt_for_year()
            project = _prompt_for_project()
            run_heatmap(year, 'project', project)
        elif choice == '4':
            year = _prompt_for_year()
            run_heatmap(year, 'sleep')
        elif choice.lower() == 'q':
            print("感谢使用，再见！")
            break
        else:
            print(f"{COLOR_RED}无效输入，请输入 1-4 之间的数字或 'q'。{COLOR_RESET}")


if __name__ == "__main__":
    main()