# barchart_generator.py

import pandas as pd
import matplotlib.pyplot as plt
import json
import argparse
from datetime import datetime, timedelta
import sys

# --- 设置 matplotlib 支持中文的字体 ---
plt.rcParams['font.sans-serif'] = ['SimHei']
plt.rcParams['axes.unicode_minus'] = False

# --- 复用原程序的数据处理和颜色常量 ---
try:
    from timeline_generator import LogicalDay, DataProcessor
    from db_access import COLOR_GREEN, COLOR_RED, COLOR_RESET
except ImportError:
    print(f"{COLOR_RED}错误：请确保此脚本与 timeline_generator.py 和 db_access.py 在同一个目录下。{COLOR_RESET}")
    sys.exit(1)

class BarChartPlotter:
    """根据处理后的数据生成并保存一个父项目时长统计的柱状图。"""
    DEFAULT_COLOR = '#CCCCCC' # 未定义颜色的项目使用的默认颜色

    # --- 新增：集中控制字体大小 ---
    # 在这里修改图表各个部分的字体大小
    FONT_SIZES = {
        'title': 22,         # 图表主标题
        'axis_label': 18,    # X轴和Y轴的标签 (例如 "总时长 (小时)")
        'tick_label': 18,    # X轴和Y轴的刻度文字 (例如 "code", "mystudy", "1.0", "2.0")
        'annotation': 16      # 每个柱状图顶部的文字 (例如 "8h 15m" 和 "38.2%")
    }
    # --- 修改结束 ---

    def __init__(self, logical_day, color_map):
        """
        初始化绘图器。
        
        参数:
            logical_day (LogicalDay): 包含当天已处理数据的对象。
            color_map (dict): 父项目到十六进制颜色代码的映射字典。
        """
        if not isinstance(logical_day, LogicalDay) or logical_day.processed_data.empty:
            raise ValueError("BarChartPlotter 必须使用一个有效的、包含已处理数据的 LogicalDay 对象进行初始化。")
        
        self.logical_day = logical_day
        self.color_map = color_map
        self.data = logical_day.processed_data.copy()

    def _prepare_summary_data(self):
        """计算每个父项目的总时长和占比。"""
        self.data['duration'] = self.data['end_dt'] - self.data['start_dt']
        duration_summary = self.data.groupby('parent')['duration'].sum().sort_values(ascending=False)
        total_day_duration = self.logical_day.end_time - self.logical_day.start_time
        summary_df = pd.DataFrame(duration_summary).reset_index()
        summary_df.columns = ['parent', 'total_duration']
        summary_df['percentage'] = summary_df['total_duration'] / total_day_duration * 100
        return summary_df

    def _format_duration(self, timedelta_obj):
        """将 timedelta 对象格式化为 'Xh Ym' 字符串。"""
        total_minutes = timedelta_obj.total_seconds() / 60
        hours = int(total_minutes // 60)
        minutes = int(total_minutes % 60)
        return f"{hours}h {minutes}m"

    def save_chart(self, output_path, title):
        """生成并保存图表。"""
        summary_data = self._prepare_summary_data()
        
        fig, ax = plt.subplots(figsize=(16, 9))
        
        colors = [self.color_map.get(cat, self.DEFAULT_COLOR) for cat in summary_data['parent']]
        
        bars = ax.bar(summary_data['parent'], summary_data['total_duration'].dt.total_seconds() / 3600, color=colors, edgecolor='black')
        
        for i, bar in enumerate(bars):
            duration_td = summary_data.loc[i, 'total_duration']
            percentage = summary_data.loc[i, 'percentage']
            duration_text = self._format_duration(duration_td)
            percentage_text = f"{percentage:.1f}%"
            label = f"{duration_text}\n{percentage_text}"
            yval = bar.get_height()
            # --- 修改：使用 FONT_SIZES 字典控制字体大小 ---
            ax.text(bar.get_x() + bar.get_width()/2.0, yval, label, ha='center', va='bottom', 
                    fontsize=self.FONT_SIZES['annotation'], color='black')

        # --- 修改：使用 FONT_SIZES 字典控制字体大小 ---
        ax.set_title(title, fontsize=self.FONT_SIZES['title'], pad=20)
        ax.set_ylabel("总时长 (小时)", fontsize=self.FONT_SIZES['axis_label'])
        ax.set_xlabel("活动大类", fontsize=self.FONT_SIZES['axis_label'])
        
        ax.grid(axis='y', linestyle='--', alpha=0.7)
        ax.set_ylim(top=ax.get_ylim()[1] * 1.15)
        
        # --- 修改：使用 FONT_SIZES 字典控制刻度文字大小 ---
        ax.tick_params(axis='x', labelsize=self.FONT_SIZES['tick_label'])
        ax.tick_params(axis='y', labelsize=self.FONT_SIZES['tick_label'])
        
        # --- 修改：移除了下面这行代码，让X轴标签保持水平 ---
        # plt.setp(ax.get_xticklabels(), rotation=45, ha="right", rotation_mode="anchor")
        
        fig.tight_layout()

        try:
            fig.savefig(output_path, bbox_inches='tight')
            print(f"{COLOR_GREEN}✅ 柱状图已成功保存至 '{output_path}'{COLOR_RESET}")
        except Exception as e:
            print(f"{COLOR_RED}❌ 保存图表时出错 '{output_path}': {e}{COLOR_RESET}")
        finally:
            plt.close(fig)

class Application:
    """程序流程的编排器。"""
    def __init__(self, date_str):
        self.date_str = date_str
        self.colors_path = 'timeline_colors_configs.json'

    def _load_color_config(self):
        """从JSON加载颜色配置，失败则退出。"""
        print(f"🎨 正在从 '{self.colors_path}' 加载颜色配置...")
        try:
            with open(self.colors_path, 'r', encoding='utf-8') as f:
                config = json.load(f)
            print(f"{COLOR_GREEN}✅ 颜色配置加载成功。{COLOR_RESET}")
            return config
        except FileNotFoundError:
            print(f"{COLOR_RED}❌ 错误: 找不到颜色配置文件 '{self.colors_path}'。{COLOR_RESET}", file=sys.stderr)
            sys.exit(1)
        except json.JSONDecodeError:
            print(f"{COLOR_RED}❌ 错误: 无法解析 '{self.colors_path}'。请检查文件语法。{COLOR_RESET}", file=sys.stderr)
            sys.exit(1)

    def run(self):
        """执行程序主逻辑。"""
        color_config = self._load_color_config()
        active_scheme_name = color_config.get('active_scheme', 'default')
        all_schemes = color_config.get('color_schemes', {})
        color_map = all_schemes.get(active_scheme_name)

        if color_map is None:
            print(f"警告: 在 '{self.colors_path}' 中找不到方案 '{active_scheme_name}'。将使用默认灰色。")
            color_map = {}

        try:
            processor = DataProcessor()
            logical_day = processor.create_logical_day(self.date_str)

            if logical_day and logical_day.processed_data is not None and not logical_day.processed_data.empty:
                plotter = BarChartPlotter(logical_day, color_map)
                output_filename = f"barchart_{self.date_str}_{active_scheme_name}.png"
                formatted_date = datetime.strptime(self.date_str, "%Y%m%d").strftime('%B %d, %Y')
                title = f"各项活动总时长分析 ({formatted_date})"
                plotter.save_chart(output_filename, title)
            elif logical_day is not None:
                 print("没有可用于绘图的已处理数据。")

        except (ValueError, ConnectionError) as e:
            print(f"{COLOR_RED}程序运行时发生错误: {e}{COLOR_RESET}")

def main():
    """主函数，用于解析参数和运行程序。"""
    parser = argparse.ArgumentParser(
        description='根据 time_data.db 数据库中的数据，为指定“逻辑日”生成一个父项目时长统计的柱状图。'
    )
    parser.add_argument(
        'date', 
        type=str, 
        help='目标查询日期，格式为 YYYYMMDD (例如: 20250528)。'
    )
    args = parser.parse_args()

    try:
        datetime.strptime(args.date, "%Y%m%d")
    except ValueError:
        print(f"{COLOR_RED}错误: 日期格式必须为 YYYYMMDD。{COLOR_RESET}")
        sys.exit(1)

    app = Application(args.date)
    app.run()

if __name__ == "__main__":
    main()