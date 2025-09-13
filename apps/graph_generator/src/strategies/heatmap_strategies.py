# graph_generator/strategies/heatmap_strategies.py
from typing import Any, Tuple, Dict
import datetime

from core.config import app_config

class HeatmapStrategy:
    """定义如何为热力图方块提供颜色和提示信息的策略接口。"""
    def get_color_and_tooltip(self, date: datetime.date, value: Any) -> Tuple[str, str]:
        raise NotImplementedError
    def get_title(self, year: int) -> str:
        raise NotImplementedError
    def get_legend(self) -> str:
        raise NotImplementedError

class NumericStrategy(HeatmapStrategy):
    """用于数值（如项目时长）数据的策略。"""
    def __init__(self, project_name: str):
        self.project_name = project_name
        config = app_config.get_heatmap_config()
        palette_name = config["DEFAULT_COLOR_PALETTE_NAME"]
        self.color_palette = config["COLOR_PALETTES"][palette_name]
        over_12h_ref = config["OVER_12_HOURS_COLOR_REF"]
        self.over_12h_color = config["SINGLE_COLORS"][over_12h_ref]

    def get_color_and_tooltip(self, date: datetime.date, hours: float) -> Tuple[str, str]:
        hours = hours or 0
        if hours > 12: color = self.over_12h_color
        elif hours > 10: color = self.color_palette[4]
        elif hours > 8:  color = self.color_palette[3]
        elif hours > 4:  color = self.color_palette[2]
        elif hours > 0:  color = self.color_palette[1]
        else:            color = self.color_palette[0]
        tooltip = f"{hours:.2f} 小时 {self.project_name} on {date.strftime('%Y-%m-%d')}"
        return color, tooltip

    def get_title(self, year: int) -> str:
        return f"{self.project_name.capitalize()} 热力图 - {year}"

    def get_legend(self) -> str:
        boxes = "".join([f'<div class="legend-box" style="background-color: {c};"></div>' for c in self.color_palette])
        return f'<div class="legend"><span>少</span>{boxes}<span>多</span></div>'

class BooleanStrategy(HeatmapStrategy):
    """用于布尔值（如睡眠状态）数据的策略。"""
    def get_color_and_tooltip(self, date: datetime.date, status: str) -> Tuple[str, str]:
        color, text = {'True': ('#9be9a8', "Sleep: True"), 'False': ('#e5534b', "Sleep: False")}.get(status, ('#ebedf0', "No Data"))
        tooltip = f"{text} on {date.strftime('%Y-%m-%d')}"
        return color, tooltip

    def get_title(self, year: int) -> str:
        return f"睡眠状态热力图 - {year}"

    def get_legend(self) -> str:
        return """
        <div class="legend">
            <div class="legend-box" style="background-color: #ebedf0;"></div><span>无数据</span>
            <div class="legend-box" style="background-color: #9be9a8;"></div><span>是</span>
            <div class="legend-box" style="background-color: #e5534b;"></div><span>否</span>
        </div>
        """