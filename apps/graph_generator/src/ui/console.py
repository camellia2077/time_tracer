# graph_generator/ui/console.py
import argparse
import sys
from datetime import datetime

from core.app import App
from core.config import COLOR_RED, COLOR_RESET

def run_cli():
    """
    设置并运行命令行参数解析器。
    """
    parser = argparse.ArgumentParser(description="一个集成的可视化工具。")
    subparsers = parser.add_subparsers(dest="command", required=True, help="可用的命令")

    # 时间线子命令
    p_timeline = subparsers.add_parser("timeline", help="为指定日期生成时间线图。")
    p_timeline.add_argument("date", type=str, help="目标日期 (格式: YYYYMMDD)")

    # 柱状图子命令
    p_barchart = subparsers.add_parser("barchart", help="为指定日期生成活动时长的柱状图。")
    p_barchart.add_argument("date", type=str, help="目标日期 (格式: YYYYMMDD)")
    
    # 项目热力图子命令
    p_heatmap = subparsers.add_parser("heatmap", help="为指定项目生成年度热力图。")
    p_heatmap.add_argument("year", type=int, help="目标年份 (例如: 2024)")
    p_heatmap.add_argument("-p", "--project", type=str, default="mystudy", help="目标父项目 (默认: mystudy)")
    
    # 睡眠热力图子命令
    p_sleep = subparsers.add_parser("sleep", help="生成年度睡眠状态热力图。")
    p_sleep.add_argument("year", type=int, help="目标年份 (例如: 2024)")
    
    args = parser.parse_args()
    app = App()

    if args.command in ["timeline", "barchart"]:
        try:
            datetime.strptime(args.date, "%Y%m%d")
            app.generate_day_chart(args.date, args.command)
        except ValueError:
            print(f"{COLOR_RED}错误: 日期格式必须为 YYYYMMDD。{COLOR_RESET}", file=sys.stderr)
            sys.exit(1)
    elif args.command == 'heatmap':
        app.generate_heatmap(args.year, 'project', args.project.lower())
    elif args.command == 'sleep':
        app.generate_heatmap(args.year, 'sleep')