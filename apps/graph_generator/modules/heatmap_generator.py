# modules/heatmap_generator.py

import datetime
import tomllib # [核心修改] 导入 tomllib
import calendar 
from typing import Dict, List, Any, Callable, Tuple

# --- 策略接口和实现 (此部分代码保持不变) ---

class HeatmapStrategy:
    """定义如何为热力图方块提供颜色和提示信息的策略接口。"""
    def get_color_and_tooltip(self, date: datetime.date, value: Any) -> Tuple[str, str]:
        raise NotImplementedError
    
    def get_title(self, year: int) -> str:
        raise NotImplementedError

    def get_legend(self) -> str:
        raise NotImplementedError

def create_numeric_heatmap_strategy(config_path: str, project_name: str) -> HeatmapStrategy:
    """工厂函数：创建用于数值（项目时长）数据的策略。"""
    import sys
    try:
        # --- [核心修改] 使用 tomllib 加载配置 ---
        with open(config_path, 'rb') as f:
            config = tomllib.load(f)
        palette_name = config["DEFAULT_COLOR_PALETTE_NAME"]
        color_palette = config["COLOR_PALETTES"][palette_name]
        over_12h_ref = config["OVER_12_HOURS_COLOR_REF"]
        over_12h_color = config["SINGLE_COLORS"][over_12h_ref]
    except (FileNotFoundError, KeyError, tomllib.TOMLDecodeError) as e:
        print(f"❌ 错误: 加载或解析颜色配置 '{config_path}'失败: {e}", file=sys.stderr)
        sys.exit(1)

    class NumericStrategy(HeatmapStrategy):
        def get_color_and_tooltip(self, date: datetime.date, hours: float) -> Tuple[str, str]:
            if hours is None:
                hours = 0
            
            if hours > 12: color = over_12h_color
            elif hours > 10: color = color_palette[4]
            elif hours > 8:  color = color_palette[3]
            elif hours > 4:  color = color_palette[2]
            elif hours > 0:  color = color_palette[1]
            else:            color = color_palette[0]

            tooltip = f"{hours:.2f} 小时 {project_name} on {date.strftime('%Y-%m-%d')}"
            return color, tooltip
            
        def get_title(self, year: int) -> str:
            return f"{project_name.capitalize()} 热力图 - {year}"

        def get_legend(self) -> str:
            boxes_html = "".join([f'<div class="legend-box" style="background-color: {color};"></div>' for color in color_palette])
            return f"""
            <div class="legend">
                <span>少</span>
                {boxes_html}
                <span>多</span>
            </div>
            """

    return NumericStrategy()

def create_boolean_heatmap_strategy() -> HeatmapStrategy:
    """工厂函数：创建用于布尔（睡眠）数据的策略。"""
    class BooleanStrategy(HeatmapStrategy):
        def get_color_and_tooltip(self, date: datetime.date, status: str) -> Tuple[str, str]:
            color, status_text = {
                'True': ('#9be9a8', "Sleep: True"),
                'False': ('#e5534b', "Sleep: False"),
            }.get(status, ('#ebedf0', "No Data"))
            tooltip = f"{status_text} on {date.strftime('%Y-%m-%d')}"
            return color, tooltip
        
        def get_title(self, year: int) -> str:
            return f"睡眠状态热力图 - {year}"

        def get_legend(self) -> str:
            return """
            <div class="legend">
                <span>Fewer</span>
                <div class="legend-box" style="background-color: #ebedf0;"></div>
                <div class="legend-box" style="background-color: #9be9a8;"></div>
                <div class="legend-box" style="background-color: #e5534b;"></div>
                <span>More</span>
            </div>
            """
    return BooleanStrategy()


# --- 通用热力图生成器 (逻辑不变) ---
class HeatmapGenerator:
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

    def save_annual_heatmap(self, filename: str) -> None:
        svg_content = self._generate_annual_svg_content()
        container_html = f"""
<div class="annual-container" style="border: 1px solid #e1e4e8; border-radius: 6px; padding: 20px; background-color: #ffffff;">
    {svg_content}
    {self.strategy.get_legend()}
</div>"""
        html_output = self._generate_html_shell(container_html)
        try:
            with open(filename, 'w', encoding='utf-8') as f:
                f.write(html_output)
            print(f"成功创建年度热力图 '{filename}'.")
        except IOError as e:
            print(f"写入文件时出错: {e}")

    def save_monthly_heatmap(self, filename: str) -> None:
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
        try:
            with open(filename, 'w', encoding='utf-8') as f:
                f.write(html_output)
            print(f"成功创建月度热力图 '{filename}'.")
        except IOError as e:
            print(f"写入文件时出错: {e}")