import argparse
import sys
from datetime import datetime

# 从重构后的模块中导入核心功能
from modules.day_analyzer import DataProcessor
from modules.plotters import TimelinePlotter, BarChartPlotter
from modules.heatmap_generator import HeatmapGenerator, create_numeric_heatmap_strategy, create_boolean_heatmap_strategy
import db_access
from db_access import COLOR_RED, COLOR_RESET, COLOR_GREEN, COLOR_YELLOW

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
    
    # 1. 加载颜色配置
    print(f"{COLOR_YELLOW}加载颜色配置...{COLOR_RESET}")
    colors_path = 'configs/timeline_colors.json'
    color_config = load_json_config(colors_path)
    active_scheme_name = color_config.get('active_scheme', 'default')
    color_map = color_config.get('color_schemes', {}).get(active_scheme_name, {})
    print(f"{COLOR_GREEN}✅ 颜色配置加载成功，使用色彩方案: '{active_scheme_name}'。{COLOR_RESET}")
    
    # 2. 处理数据
    print(f"{COLOR_YELLOW}正在连接数据库并准备数据处理器...{COLOR_RESET}")
    try:
        processor = DataProcessor()
        print(f"{COLOR_GREEN}✅ 数据库连接成功，数据处理器已就绪。{COLOR_RESET}")
        
        print(f"{COLOR_YELLOW}正在为日期 {date_str} 创建逻辑日并处理数据...{COLOR_RESET}")
        logical_day = processor.create_logical_day(date_str)
        
    except ConnectionError as e:
        print(f"{COLOR_RED}❌ 数据库连接失败: {e}{COLOR_RESET}", file=sys.stderr)
        return

    if not logical_day or logical_day.processed_data.empty:
        print(f"{COLOR_RED}❌ 未能找到或处理该日期的数据，无法生成图表。{COLOR_RESET}")
        return
    
    print(f"{COLOR_GREEN}✅ 数据处理成功，找到 {len(logical_day.processed_data)} 条记录。{COLOR_RESET}")

    # 3. 根据类型选择绘图器并生成图表
    formatted_date = datetime.strptime(date_str, "%Y%m%d").strftime('%B %d, %Y')
    
    print(f"{COLOR_YELLOW}准备开始绘制图表...{COLOR_RESET}")
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
    data = None

    if heatmap_type == 'project':
        # 1. 获取项目数据和策略
        print(f"{COLOR_YELLOW}正在从数据库获取项目 '{project}' 在 {year} 年的数据...{COLOR_RESET}")
        data = db_access.get_data_for_heatmap(year, project)
        if not data:
            print(f"{COLOR_RED}❌ 未在数据库中找到项目 '{project}' 的任何数据。{COLOR_RESET}", file=sys.stderr)
            return
        print(f"{COLOR_GREEN}✅ 成功获取 {len(data)} 条项目数据。{COLOR_RESET}")

        print(f"{COLOR_YELLOW}正在创建数值型热力图的颜色策略...{COLOR_RESET}")
        strategy = create_numeric_heatmap_strategy('configs/heatmap_colors.json', project) 
        print(f"{COLOR_GREEN}✅ 颜色策略创建成功。{COLOR_RESET}")
        base_filename = f"heatmap_{project}_{year}"

    elif heatmap_type == 'sleep':
        # 2. 获取睡眠数据和策略
        print(f"{COLOR_YELLOW}正在从数据库获取 {year} 年的睡眠数据...{COLOR_RESET}")
        data = db_access.get_sleep_data_for_bool_heatmap(year)
        if data is None:
             print(f"{COLOR_RED}❌ 无法获取 {year} 年的睡眠数据。{COLOR_RESET}", file=sys.stderr)
             return
        print(f"{COLOR_GREEN}✅ 成功获取睡眠数据。{COLOR_RESET}")

        print(f"{COLOR_YELLOW}正在创建布尔型热力图的颜色策略...{COLOR_RESET}")
        strategy = create_boolean_heatmap_strategy()
        print(f"{COLOR_GREEN}✅ 颜色策略创建成功。{COLOR_RESET}")
        base_filename = f"{year}_sleep_heatmap"
    
    # 3. 实例化生成器
    print(f"{COLOR_YELLOW}正在实例化热力图生成器...{COLOR_RESET}")
    generator = HeatmapGenerator(year, data, strategy)
    print(f"{COLOR_GREEN}✅ 热力图生成器已准备就绪。{COLOR_RESET}")
    
    # 4. 生成并保存两种格式的图表
    annual_filename = f"{base_filename}_annual.html"
    monthly_filename = f"{base_filename}_monthly.html"

    print(f"{COLOR_YELLOW}正在生成年度视图热力图...{COLOR_RESET}")
    generator.save_annual_heatmap(annual_filename)
    print(f"{COLOR_GREEN}  -> 年度视图已保存到: {annual_filename}{COLOR_RESET}")
    
    print(f"{COLOR_YELLOW}正在生成月度视图热力图...{COLOR_RESET}")
    generator.save_monthly_heatmap(monthly_filename)
    print(f"{COLOR_GREEN}  -> 月度视图已保存到: {monthly_filename}{COLOR_RESET}")
    
    print(f"\n🎉 {COLOR_GREEN}成功生成两种热力图!{COLOR_RESET}")


def main():
    # 步骤 1: 在函数顶部定义版本和日期信息
    APP_VERSION = "0.0.1"
    LAST_UPDATE = "2025-06-24"
    
    parser = argparse.ArgumentParser(
        description="一个集成的可视化工具，用于从 time_data.db 生成图表。",
        formatter_class=argparse.RawTextHelpFormatter
    )

    # 步骤 2: 添加 --version 参数
    parser.add_argument(
        '-v', '--version', 
        action='version', 
        version=f'%(prog)s {APP_VERSION} (Last Updated: {LAST_UPDATE})'
    )

    subparsers = parser.add_subparsers(dest="command", help="可用的命令")

    # --- 时间线子命令 ---
    parser_timeline = subparsers.add_parser("timeline", help="为指定日期生成时间线图。")
    parser_timeline.add_argument("date", type=str, help="目标日期 (格式: YYYYMMDD)")

    # --- 柱状图子命令 ---
    parser_barchart = subparsers.add_parser("barchart", help="为指定日期生成活动时长的柱状图。")
    parser_barchart.add_argument("date", type=str, help="目标日期 (格式: YYYYMMDD)")

    # --- 项目热力图子命令 ---
    parser_heatmap = subparsers.add_parser("heatmap", help="为指定项目生成年度和月度提交热力图。")
    parser_heatmap.add_argument("year", type=int, help="目标年份 (例如: 2024)")
    parser_heatmap.add_argument("-p", "--project", type=str, default="mystudy", help="目标父项目名称 (默认: mystudy)")

    # --- 睡眠热力图子命令 ---
    parser_sleep = subparsers.add_parser("sleep", help="生成年度和月度睡眠状态热力图。")
    parser_sleep.add_argument("year", type=int, help="目标年份 (例如: 2024)")

    args = parser.parse_args()

    if not args.command:
        parser.print_help(sys.stderr)
        sys.exit(1)

    if args.command in ["timeline", "barchart"]:
        try:
            datetime.strptime(args.date, "%Y%m%d")
            run_day_analysis_chart(args.date, args.command)
        except ValueError:
            print(f"{COLOR_RED}错误: 日期格式必须为 YYYYMMDD。{COLOR_RESET}", file=sys.stderr)
            sys.exit(1)
    elif args.command == 'heatmap':
        run_heatmap(args.year, 'project', args.project.lower())
    elif args.command == 'sleep':
        run_heatmap(args.year, 'sleep')

if __name__ == '__main__':
    main()
