# modules/heatmap_generator.py

import datetime
import json
from typing import Dict, List, Any, Callable, Tuple

# --- 策略接口和实现 ---

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
        with open(config_path, 'r', encoding='utf-8') as f:
            config = json.load(f)
        palette_name = config["DEFAULT_COLOR_PALETTE_NAME"]
        color_palette = config["COLOR_PALETTES"][palette_name]
        over_12h_ref = config["OVER_12_HOURS_COLOR_REF"]
        over_12h_color = config["SINGLE_COLORS"][over_12h_ref]
    except (FileNotFoundError, KeyError) as e:
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


# --- 通用热力图生成器 ---

class HeatmapGenerator:
    """一个通用的、用于生成年度热力图HTML文件的类。"""
    SQUARE_SIZE, SQUARE_GAP = 12, 3
    LEFT_PADDING, TOP_PADDING = 30, 30

    def __init__(self, year: int, data: Dict[datetime.date, Any], strategy: HeatmapStrategy):
        self.year = year
        self.data = data
        self.strategy = strategy
        self.GRID_UNIT = self.SQUARE_SIZE + self.SQUARE_GAP
        
        start_date = datetime.date(self.year, 1, 1)
        total_days = (datetime.date(self.year, 12, 31) - start_date).days + 1
        first_day_weekday = start_date.isoweekday() % 7
        num_weeks = (total_days + first_day_weekday + 6) // 7
        self.svg_width = num_weeks * self.GRID_UNIT + self.LEFT_PADDING
        self.svg_height = 7 * self.GRID_UNIT + self.TOP_PADDING

    def _generate_day_rects(self) -> List[str]:
        rects_svg = []
        start_date = datetime.date(self.year, 1, 1)
        total_days = 366 if (self.year % 4 == 0 and self.year % 100 != 0) or (self.year % 400 == 0) else 365
        first_day_weekday = start_date.isoweekday() % 7

        for day_index in range(total_days):
            current_date = start_date + datetime.timedelta(days=day_index)
            value = self.data.get(current_date)
            
            grid_day_index = day_index + first_day_weekday
            week_x = grid_day_index // 7
            day_y = grid_day_index % 7
            
            x_pos = week_x * self.GRID_UNIT + self.LEFT_PADDING
            y_pos = day_y * self.GRID_UNIT + self.TOP_PADDING
            
            color, tooltip = self.strategy.get_color_and_tooltip(current_date, value)
            
            rects_svg.append(
                f'    <rect width="{self.SQUARE_SIZE}" height="{self.SQUARE_SIZE}" x="{x_pos}" y="{y_pos}" '
                f'fill="{color}" rx="2" ry="2"><title>{tooltip}</title></rect>'
            )
        return rects_svg

    def _generate_month_labels(self) -> List[str]:
        month_labels_svg, last_month = [], -1
        start_date = datetime.date(self.year, 1, 1)
        total_days = (datetime.date(self.year, 12, 31) - start_date).days + 1
        first_day_weekday = start_date.isoweekday() % 7
        for day_index in range(total_days):
            current_date = start_date + datetime.timedelta(days=day_index)
            if current_date.month != last_month:
                week_x = (day_index + first_day_weekday) // 7
                x_pos = week_x * self.GRID_UNIT + self.LEFT_PADDING
                month_labels_svg.append(f'  <text x="{x_pos}" y="{self.TOP_PADDING - 10}" class="month">{current_date.strftime("%b")}</text>')
                last_month = current_date.month
        return month_labels_svg

    def _generate_weekday_labels(self) -> List[str]:
        return [
            f'<text x="{self.LEFT_PADDING - 15}" y="{self.TOP_PADDING + self.GRID_UNIT * i + self.SQUARE_SIZE / 1.5}" class="day-label">{label}</text>'
            for i, label in zip([1, 3, 5], ["Mon", "Wed", "Fri"])
        ]

    def generate_heatmap_html(self) -> str:
        svg_content = f"""
<svg width="{self.svg_width}" height="{self.svg_height}">
    <g>
        {"\n".join(self._generate_month_labels())}
        {"\n".join(self._generate_weekday_labels())}
        {"\n".join(self._generate_day_rects())}
    </g>
</svg>
"""
        return f"""
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <title>{self.strategy.get_title(self.year)}</title>
    <style>
        body {{ font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif; display: flex; flex-direction: column; align-items: center; margin: 2em; }}
        .heatmap-container {{ border: 1px solid #e1e4e8; border-radius: 6px; padding: 20px; background-color: #ffffff; }}
        h1 {{ font-weight: 400; }}
        .month, .day-label {{ font-size: 10px; fill: #586069; }}
        rect:hover {{ stroke: #24292e; stroke-width: 1.5px; }}
        .legend {{ display: flex; justify-content: flex-end; align-items: center; gap: 5px; margin-top: 10px; font-size: 12px; }}
        .legend-box {{ width: 12px; height: 12px; border-radius: 2px; }}
    </style>
</head>
<body>
    <h1>{self.strategy.get_title(self.year)}</h1>
    <div class="heatmap-container">
        {svg_content}
        {self.strategy.get_legend()}
    </div>
</body>
</html>
"""

    def save_to_file(self, filename: str) -> None:
        html_content = self.generate_heatmap_html()
        try:
            with open(filename, 'w', encoding='utf-8') as f:
                f.write(html_content)
            print(f"成功创建文件 '{filename}'.")
        except IOError as e:
            print(f"写入文件时出错: {e}")