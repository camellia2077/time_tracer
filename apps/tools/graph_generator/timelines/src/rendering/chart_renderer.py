import os
from datetime import datetime
from typing import List, Tuple, Dict
import matplotlib.pyplot as plt
import matplotlib.dates as mdates

class ChartRenderer:
    """负责将时间线数据渲染成图表并保存。"""
    def __init__(self, output_dir: str):
        self.output_dir = output_dir
        self._setup_matplotlib()

    def _setup_matplotlib(self):
        """配置 Matplotlib 的字体以支持中文。"""
        plt.rcParams['font.family'] = 'sans-serif'
        plt.rcParams['font.sans-serif'] = ['SimHei', 'Microsoft YaHei'] # 提供备用字体
        plt.rcParams['axes.unicode_minus'] = False

    def render(self, records: List[Tuple], colors: Dict[str, str], target_date: str):
        """根据数据和颜色配置生成图表。"""
        fig, ax = plt.subplots(figsize=(18, 10))
        y_labels, y_positions = [], []
        default_color = colors.get("default", "#808080")

        for i, record in enumerate(records):
            start_ts, end_ts, project_path = record
            
            start_dt = datetime.fromtimestamp(start_ts)
            end_dt = datetime.fromtimestamp(end_ts)
            
            bar_color = self._get_activity_color(project_path, colors, default_color)
            
            # 绘制条形
            start_num, end_num = mdates.date2num(start_dt), mdates.date2num(end_dt)
            ax.broken_barh([(start_num, end_num - start_num)], (i - 0.4, 0.8), facecolors=bar_color, edgecolors='grey')
            
            # 添加时长文本
            duration_seconds = end_ts - start_ts
            duration_h = int(duration_seconds / 3600)
            duration_m = int((duration_seconds % 3600) / 60)
            duration_text = f"{duration_h}h {duration_m}m"
            ax.text(start_num + (end_num - start_num) / 2, i, duration_text, ha='center', va='center', color='black', fontsize=9, weight='bold')

            y_labels.append(project_path)
            y_positions.append(i)

        self._format_chart(ax, y_positions, y_labels, len(records), target_date)
        self._save_chart(target_date)

    def _get_activity_color(self, project_path: str, color_map: Dict[str, str], default: str) -> str:
        """根据 project_path 的顶级父级获取颜色。"""
        top_parent = project_path.split('_')[0]
        return color_map.get(top_parent, default)
        
    def _format_chart(self, ax, y_pos, y_labels, num_records, date_str):
        """格式化图表的坐标轴、标题等。"""
        ax.set_yticks(y_pos)
        ax.set_yticklabels(y_labels, fontsize=12)
        ax.set_ylim(-0.5, num_records - 0.5)
        ax.set_ylabel('活动名称 (Project Path)', fontsize=14, weight='bold')

        ax.xaxis_date()
        ax.xaxis.set_major_formatter(mdates.DateFormatter('%H:%M'))
        ax.xaxis.set_major_locator(plt.MaxNLocator(24))
        plt.setp(ax.get_xticklabels(), rotation=45, ha="right", fontsize=10)

        ax.set_xlabel('一天内的时间', fontsize=14, weight='bold')
        plt.title(f'{date_str} 活动时间线', fontsize=18, weight='bold')
        plt.grid(True, which='major', axis='x', linestyle='--', linewidth=0.7)
        plt.tight_layout()

    def _save_chart(self, target_date: str):
        """保存图表到文件。"""
        if not os.path.exists(self.output_dir):
            os.makedirs(self.output_dir)
            print(f"已创建输出目录: {self.output_dir}")

        output_filename = f"{target_date}_timeline.png"
        output_path = os.path.join(self.output_dir, output_filename)
        
        plt.savefig(output_path, dpi=300)
        print(f"图表已成功保存到: {output_path}")