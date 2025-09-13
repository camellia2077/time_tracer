# graph_generator/strategies/heatmap_strategies.py
from typing import Any, Tuple, Dict
import datetime

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
    # ✨ 核心修改: __init__ 现在接收颜色配置
    def __init__(self, project_name: str, color_config: Dict[str, Any]):
        self.project_name = project_name
        self.color_palette = color_config['palette']
        self.over_12h_color = color_config['over_12h']

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
        # 将项目名首字母大写
        display_name = self.project_name.replace("_", " ").capitalize()
        return f"{display_name} - {year}年度活动热力图"

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