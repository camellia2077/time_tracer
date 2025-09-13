# graph_generator/rendering/plotters.py
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.dates as mdates

# --- [核心修改] 将相对导入改为绝对导入 ---
from data.day_analyzer import LogicalDay
from core.config import COLOR_RED, COLOR_RESET

# 设置 matplotlib 支持中文
plt.rcParams['font.sans-serif'] = ['SimHei']
plt.rcParams['axes.unicode_minus'] = False

class TimelinePlotter:
    """从已处理的数据创建时间线可视化图表。"""
    DEFAULT_COLOR = '#CCCCCC'

    def __init__(self, logical_day, color_map):
        if not isinstance(logical_day, LogicalDay) or logical_day.processed_data.empty:
            raise ValueError("TimelinePlotter 必须使用一个有效的、包含已处理数据的 LogicalDay 对象进行初始化。")
        self.data = logical_day.processed_data
        self.start_dt = logical_day.start_time
        self.end_dt = logical_day.end_time
        self.color_map = color_map
        
    def save_chart(self, output_path, title):
        parent_categories = sorted(self.data['parent'].unique())
        y_labels = {cat: i for i, cat in enumerate(parent_categories)}
        cat_colors = {cat: self.color_map.get(cat, self.DEFAULT_COLOR) for cat in parent_categories}
        
        fig, ax = plt.subplots(figsize=(15, 8))
        for _, row in self.data.iterrows():
            y_pos = y_labels[row['parent']]
            start = mdates.date2num(row['start_dt'])
            end = mdates.date2num(row['end_dt'])
            duration = end - start
            ax.barh(y_pos, duration, left=start, height=0.6, color=cat_colors.get(row['parent']), edgecolor='black', linewidth=0.5)
            
        ax.set_yticks(list(y_labels.values()))
        ax.set_yticklabels(list(y_labels.keys()))
        ax.invert_yaxis()
        ax.xaxis.set_major_formatter(mdates.DateFormatter('%H:%M'))
        ax.xaxis.set_major_locator(mdates.HourLocator(interval=1))
        ax.set_xlim(self.start_dt, self.end_dt)
        plt.setp(ax.get_xticklabels(), rotation=45, ha="right")
        ax.grid(axis='x', linestyle='--', alpha=0.6)
        ax.set_title(title, fontsize=16)
        ax.set_xlabel("时间")
        ax.set_ylabel("活动大类")
        
        try:
            fig.savefig(output_path, bbox_inches='tight')
        except Exception as e:
            print(f"{COLOR_RED}保存图表时出错 '{output_path}': {e}{COLOR_RESET}")
        finally:
            plt.close(fig)

class BarChartPlotter:
    """生成并保存一个父项目时长统计的柱状图。"""
    DEFAULT_COLOR = '#CCCCCC'
    FONT_SIZES = {'title': 22, 'axis_label': 18, 'tick_label': 18, 'annotation': 16}

    def __init__(self, logical_day, color_map):
        if not isinstance(logical_day, LogicalDay) or logical_day.processed_data.empty:
            raise ValueError("BarChartPlotter 必须使用一个有效的、包含已处理数据的 LogicalDay 对象进行初始化。")
        self.logical_day = logical_day
        self.color_map = color_map
        self.data = logical_day.processed_data.copy()

    def _prepare_summary_data(self):
        self.data['duration'] = self.data['end_dt'] - self.data['start_dt']
        duration_summary = self.data.groupby('parent')['duration'].sum().sort_values(ascending=False)
        total_day_duration = self.logical_day.end_time - self.logical_day.start_time
        summary_df = pd.DataFrame(duration_summary).reset_index()
        summary_df.columns = ['parent', 'total_duration']
        summary_df['percentage'] = summary_df['total_duration'] / total_day_duration * 100
        return summary_df

    def _format_duration(self, timedelta_obj):
        total_minutes = timedelta_obj.total_seconds() / 60
        hours = int(total_minutes // 60)
        minutes = int(total_minutes % 60)
        return f"{hours}h {minutes}m"

    def save_chart(self, output_path, title):
        summary_data = self._prepare_summary_data()
        fig, ax = plt.subplots(figsize=(16, 9))
        colors = [self.color_map.get(cat, self.DEFAULT_COLOR) for cat in summary_data['parent']]
        bars = ax.bar(summary_data['parent'], summary_data['total_duration'].dt.total_seconds() / 3600, color=colors, edgecolor='black')
        
        for i, bar in enumerate(bars):
            duration_td = summary_data.loc[i, 'total_duration']
            percentage = summary_data.loc[i, 'percentage']
            label = f"{self._format_duration(duration_td)}\n{percentage:.1f}%"
            ax.text(bar.get_x() + bar.get_width()/2.0, bar.get_height(), label, ha='center', va='bottom', fontsize=self.FONT_SIZES['annotation'])

        ax.set_title(title, fontsize=self.FONT_SIZES['title'], pad=20)
        ax.set_ylabel("总时长 (小时)", fontsize=self.FONT_SIZES['axis_label'])
        ax.set_xlabel("活动大类", fontsize=self.FONT_SIZES['axis_label'])
        ax.grid(axis='y', linestyle='--', alpha=0.7)
        ax.set_ylim(top=ax.get_ylim()[1] * 1.15)
        ax.tick_params(axis='x', labelsize=self.FONT_SIZES['tick_label'])
        ax.tick_params(axis='y', labelsize=self.FONT_SIZES['tick_label'])
        fig.tight_layout()

        try:
            fig.savefig(output_path, bbox_inches='tight')
        except Exception as e:
            print(f"{COLOR_RED}保存图表时出错 '{output_path}': {e}{COLOR_RESET}")
        finally:
            plt.close(fig)