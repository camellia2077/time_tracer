import argparse
import datetime
import os
from typing import Dict, List, Optional, Tuple
from db_access import get_sleep_data_for_bool_heatmap # <-- 数据访问保持不变

class HeatmapGenerator:
    """
    一个用于生成和管理年度睡眠状态热力图HTML文件的类。
    """
    # --- 类常量，定义SVG样式和布局 ---
    SQUARE_SIZE, SQUARE_GAP = 12, 3
    GRID_UNIT = SQUARE_SIZE + SQUARE_GAP
    LEFT_PADDING, TOP_PADDING = 30, 30

    def __init__(self, year: int, sleep_data: Dict[datetime.date, str]):
        """
        初始化热力图生成器。

        Args:
            year (int): 热力图的目标年份。
            sleep_data (Dict[datetime.date, str]): 包含日期和睡眠状态的字典。
        """
        self.year = year
        self.sleep_data = sleep_data
        
        # 计算并存储SVG的尺寸
        start_date = datetime.date(self.year, 1, 1)
        total_days = (datetime.date(self.year, 12, 31) - start_date).days + 1
        first_day_weekday = start_date.isoweekday() % 7
        num_weeks = (total_days + first_day_weekday + 6) // 7
        self.svg_width = num_weeks * self.GRID_UNIT + self.LEFT_PADDING
        self.svg_height = 7 * self.GRID_UNIT + self.TOP_PADDING

    def _generate_day_rects(self) -> List[str]:
        """(内部方法) 生成代表每一天的SVG <rect> 元素。"""
        rects_svg = []
        start_date = datetime.date(self.year, 1, 1)
        total_days = (datetime.date(self.year, 12, 31) - start_date).days + 1
        first_day_weekday = start_date.isoweekday() % 7

        for day_index in range(total_days):
            current_date = start_date + datetime.timedelta(days=day_index)
            grid_day_index = day_index + first_day_weekday
            week_x = grid_day_index // 7
            day_y = grid_day_index % 7
            
            x_pos = week_x * self.GRID_UNIT + self.LEFT_PADDING
            y_pos = day_y * self.GRID_UNIT + self.TOP_PADDING

            sleep_status = self.sleep_data.get(current_date, 'N/A')
            color, status_text = {
                'True': ('#9be9a8', "Sleep: True"),
                'False': ('#e5534b', "Sleep: False"),
            }.get(sleep_status, ('#ebedf0', "No Data"))
            
            tooltip = f"{status_text} on {current_date.strftime('%Y-%m-%d')}"
            
            rects_svg.append(
                f'    <rect width="{self.SQUARE_SIZE}" height="{self.SQUARE_SIZE}" x="{x_pos}" y="{y_pos}" '
                f'fill="{color}" rx="2" ry="2"><title>{tooltip}</title></rect>'
            )
        return rects_svg

    def _generate_month_labels(self) -> List[str]:
        """(内部方法) 生成月份标签的SVG <text> 元素。"""
        month_labels_svg = []
        last_month = -1
        start_date = datetime.date(self.year, 1, 1)
        total_days = (datetime.date(self.year, 12, 31) - start_date).days + 1
        first_day_weekday = start_date.isoweekday() % 7

        for day_index in range(total_days):
            current_date = start_date + datetime.timedelta(days=day_index)
            if current_date.month != last_month:
                grid_day_index = day_index + first_day_weekday
                week_x = grid_day_index // 7
                x_pos = week_x * self.GRID_UNIT + self.LEFT_PADDING
                month_labels_svg.append(
                    f'  <text x="{x_pos}" y="{self.TOP_PADDING - 10}" class="month">{current_date.strftime("%b")}</text>'
                )
                last_month = current_date.month
        return month_labels_svg

    def _generate_weekday_labels(self) -> List[str]:
        """(内部方法) 生成星期标签的SVG <text> 元素。"""
        day_labels = ["Mon", "Wed", "Fri"]
        day_indices = [1, 3, 5]
        day_labels_svg = []
        for i, label in zip(day_indices, day_labels):
            y_pos = self.TOP_PADDING + self.GRID_UNIT * i + self.SQUARE_SIZE / 1.5
            day_labels_svg.append(
                f'<text x="{self.LEFT_PADDING - 15}" y="{y_pos}" class="day-label">{label}</text>'
            )
        return day_labels_svg

    def _generate_svg_content(self) -> str:
        """(内部方法) 组合所有部分，生成完整的SVG内容字符串。"""
        rects_svg = self._generate_day_rects()
        month_labels_svg = self._generate_month_labels()
        day_labels_svg = self._generate_weekday_labels()

        return f"""
<svg width="{self.svg_width}" height="{self.svg_height}">
    <g>
        {"\n".join(month_labels_svg)}
        {"\n".join(day_labels_svg)}
        {"\n".join(rects_svg)}
    </g>
</svg>
"""

    def generate_heatmap_html(self) -> str:
        """生成包含热力图的完整HTML页面。"""
        svg_content = self._generate_svg_content()
        return f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Sleep Status Heatmap - {self.year}</title>
    <style>
        body {{ font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif; background-color: #f6f8fa; color: #24292e; display: flex; flex-direction: column; align-items: center; margin: 2em; }}
        .heatmap-container {{ border: 1px solid #e1e4e8; border-radius: 6px; padding: 20px; background-color: #ffffff; }}
        h1 {{ font-weight: 400; text-align: center; margin-bottom: 1em; }}
        .month, .day-label {{ font-size: 10px; fill: #586069; }}
        .day-label {{ text-anchor: middle; }}
        rect:hover {{ stroke: #24292e; stroke-width: 1.5px; }}
        .legend {{ display: flex; justify-content: flex-end; align-items: center; gap: 5px; margin-top: 10px; font-size: 12px; color: #586069; }}
        .legend span {{ margin-right: 2px; }}
        .legend-box {{ width: 12px; height: 12px; border-radius: 2px; margin-right: 8px;}}
    </style>
</head>
<body>
    <h1>Sleep Status - {self.year}</h1>
    <div class="heatmap-container">
        {svg_content}
        <div class="legend">
            <span>Fewer</span>
            <div class="legend-box" style="background-color: #ebedf0;"></div>
            <div class="legend-box" style="background-color: #9be9a8;"></div>
            <div class="legend-box" style="background-color: #e5534b;"></div>
            <span>More</span>
        </div>
    </div>
</body>
</html>
"""

    def save_to_file(self, filename: str) -> None:
        """将生成的HTML内容保存到文件。"""
        html_content = self.generate_heatmap_html()
        try:
            with open(filename, 'w', encoding='utf-8') as f:
                f.write(html_content)
            print(f"Successfully created '{filename}'.")
        except IOError as e:
            print(f"Error writing to file: {e}")

# --- Application Layer ---

def main() -> None:
    """解析参数并使用HeatmapGenerator类来编排热力图的生成。"""
    parser = argparse.ArgumentParser(
        description="Generate a GitHub-style sleep heatmap from a time_data.db file.",
        formatter_class=argparse.RawTextHelpFormatter
    )
    parser.add_argument("year", type=int, help="The year for the heatmap (e.g., 2024).")
    args = parser.parse_args()
    
    year = args.year
    output_filename = f"{year}_sleep_heatmap.html"

    print(f"Fetching data for year {year}...")
    sleep_data = get_sleep_data_for_bool_heatmap(year)
    
    if sleep_data is None:
        print("Failed to fetch data. Exiting.")
        return

    print(f"Found data for {len(sleep_data)} days.")
    print(f"Generating HTML file: {output_filename}...")

    # --- 使用新的类 ---
    # 1. 创建实例
    generator = HeatmapGenerator(year, sleep_data)
    # 2. 调用方法保存文件
    generator.save_to_file(output_filename)

if __name__ == "__main__":
    main()