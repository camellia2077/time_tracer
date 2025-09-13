# graph_generator/rendering/heatmap_renderer.py
import datetime
import calendar
from typing import Dict, Any

# 从新的策略模块中导入策略接口
from .heatmap_strategies import HeatmapStrategy 

class HeatmapRenderer:
    """
    通用热力图生成器，只负责渲染，不关心业务逻辑。
    它接收一个包含颜色和提示信息决策的“策略”对象来完成工作。
    """
    # --- 常量定义 ---
    ANNUAL_SQUARE_SIZE, ANNUAL_SQUARE_GAP = 12, 3
    ANNUAL_LEFT_PADDING, ANNUAL_TOP_PADDING = 30, 30
    MONTHLY_SQUARE_SIZE, MONTHLY_SQUARE_GAP = 15, 4
    MONTHLY_LEFT_PADDING, MONTHLY_TOP_PADDING = 15, 40
    MONTHLY_HEADER_HEIGHT = 30

    def __init__(self, year: int, data: Dict[datetime.date, Any], strategy: HeatmapStrategy):
        self.year = year
        self.data = data
        self.strategy = strategy
        self.cal = calendar.Calendar()

    def _generate_annual_svg_content(self) -> str:
        """生成年度视图的SVG核心内容。"""
        GRID_UNIT = self.ANNUAL_SQUARE_SIZE + self.ANNUAL_SQUARE_GAP
        start_date = datetime.date(self.year, 1, 1)
        total_days = (datetime.date(self.year, 12, 31) - start_date).days + 1
        first_day_weekday = start_date.isoweekday() % 7
        num_weeks = (total_days + first_day_weekday + 6) // 7
        svg_width = num_weeks * GRID_UNIT + self.ANNUAL_LEFT_PADDING
        svg_height = 7 * GRID_UNIT + self.ANNUAL_TOP_PADDING

        rects_svg, month_labels_svg, day_labels_svg = [], [], []
        last_month = -1
        
        for i, label in zip([1, 3, 5], ["Mon", "Wed", "Fri"]):
             y_pos = self.ANNUAL_TOP_PADDING + GRID_UNIT * i + self.ANNUAL_SQUARE_SIZE / 1.5
             day_labels_svg.append(f'<text x="{self.ANNUAL_LEFT_PADDING - 15}" y="{y_pos}" class="day-label">{label}</text>')
        
        for day_index in range(total_days):
            current_date = start_date + datetime.timedelta(days=day_index)
            value = self.data.get(current_date)
            grid_day_index = day_index + first_day_weekday
            week_x, day_y = divmod(grid_day_index, 7)
            x_pos = week_x * GRID_UNIT + self.ANNUAL_LEFT_PADDING
            y_pos = day_y * GRID_UNIT + self.ANNUAL_TOP_PADDING
            
            color, tooltip = self.strategy.get_color_and_tooltip(current_date, value)
            rects_svg.append(f'    <rect width="{self.ANNUAL_SQUARE_SIZE}" height="{self.ANNUAL_SQUARE_SIZE}" x="{x_pos}" y="{y_pos}" fill="{color}" rx="2" ry="2"><title>{tooltip}</title></rect>')
            
            if current_date.month != last_month:
                month_labels_svg.append(f'  <text x="{x_pos}" y="{self.ANNUAL_TOP_PADDING - 10}" class="month">{current_date.strftime("%b")}</text>')
                last_month = current_date.month

        return f"""
<svg width="{svg_width}" height="{svg_height}">
    <g>
        {"\n".join(month_labels_svg)}
        {"\n".join(day_labels_svg)}
        {"\n".join(rects_svg)}
    </g>
</svg>"""

    def _generate_single_month_svg(self, month: int) -> str:
        """为单个月份生成SVG内容块。"""
        GRID_UNIT = self.MONTHLY_SQUARE_SIZE + self.MONTHLY_SQUARE_GAP
        month_weeks = self.cal.monthdatescalendar(self.year, month)
        
        svg_width = 7 * GRID_UNIT + self.MONTHLY_LEFT_PADDING * 2
        svg_height = len(month_weeks) * GRID_UNIT + self.MONTHLY_TOP_PADDING + self.MONTHLY_HEADER_HEIGHT

        rects_svg, weekday_labels_svg = [], []
        month_name = datetime.date(self.year, month, 1).strftime('%B')
        
        for i, day_name in enumerate(["S", "M", "T", "W", "T", "F", "S"]):
            x_pos = i * GRID_UNIT + self.MONTHLY_LEFT_PADDING + self.MONTHLY_SQUARE_SIZE / 2
            weekday_labels_svg.append(f'<text x="{x_pos}" y="{self.MONTHLY_TOP_PADDING - 10}" class="day-label" text-anchor="middle">{day_name}</text>')

        for week_index, week in enumerate(month_weeks):
            for day_index, current_date in enumerate(week):
                if current_date.month != month:
                    continue
                
                value = self.data.get(current_date)
                x_pos = day_index * GRID_UNIT + self.MONTHLY_LEFT_PADDING
                y_pos = week_index * GRID_UNIT + self.MONTHLY_TOP_PADDING
                
                color, tooltip = self.strategy.get_color_and_tooltip(current_date, value)
                rects_svg.append(f'    <rect width="{self.MONTHLY_SQUARE_SIZE}" height="{self.MONTHLY_SQUARE_SIZE}" x="{x_pos}" y="{y_pos}" fill="{color}" rx="3" ry="3"><title>{tooltip}</title></rect>')

        return f"""
<div class="month-container">
    <h3 class="month-title">{month_name}</h3>
    <svg width="{svg_width}" height="{svg_height}">
        <g>
            {"\n".join(weekday_labels_svg)}
            {"\n".join(rects_svg)}
        </g>
    </svg>
</div>
"""

    def _generate_html_shell(self, content: str, custom_style: str = "") -> str:
        """生成HTML文件的外壳模板。"""
        return f"""
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <title>{self.strategy.get_title(self.year)}</title>
    <style>
        body {{ font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif; display: flex; flex-direction: column; align-items: center; margin: 2em; background-color: #f6f8fa; }}
        h1 {{ font-weight: 400; text-align: center; }}
        .month, .day-label {{ font-size: 10px; fill: #586069; }}
        rect:hover {{ stroke: #24292e; stroke-width: 1.5px; }}
        .legend {{ display: flex; justify-content: flex-end; align-items: center; gap: 5px; margin-top: 10px; font-size: 12px; }}
        .legend-box {{ width: 12px; height: 12px; border-radius: 2px; }}
        {custom_style}
    </style>
</head>
<body>
    <h1>{self.strategy.get_title(self.year)}</h1>
    {content}
</body>
</html>
"""

    def _write_file(self, filename: str, content: str):
        """将内容写入文件，包含错误处理。"""
        try:
            with open(filename, 'w', encoding='utf-8') as f:
                f.write(content)
        except IOError as e:
            print(f"写入文件时出错 '{filename}': {e}")

    def save_annual_heatmap(self, filename: str):
        """生成并保存年度热力图。"""
        svg_content = self._generate_annual_svg_content()
        container_html = f"""
<div class="annual-container" style="border: 1px solid #e1e4e8; border-radius: 6px; padding: 20px; background-color: #ffffff;">
    {svg_content}
    {self.strategy.get_legend()}
</div>"""
        html_output = self._generate_html_shell(container_html)
        self._write_file(filename, html_output)

    def save_monthly_heatmap(self, filename: str):
        """生成并保存月度热力图。"""
        all_months_svg = [self._generate_single_month_svg(m) for m in range(1, 13)]
        
        monthly_style = """
        .months-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(250px, 1fr)); gap: 20px; width: 100%; max-width: 1200px; }
        .month-container { border: 1px solid #e1e4e8; border-radius: 6px; padding: 15px; background-color: #ffffff; }
        .month-title { text-align: center; margin-top: 0; margin-bottom: 15px; font-weight: 500; }
        """
        
        container_html = f"""
<div class="months-grid">
    {''.join(all_months_svg)}
</div>
<div style="margin-top: 20px; width: 100%; max-width: 1200px;">{self.strategy.get_legend()}</div>
"""
        html_output = self._generate_html_shell(container_html, custom_style=monthly_style)
        self._write_file(filename, html_output)