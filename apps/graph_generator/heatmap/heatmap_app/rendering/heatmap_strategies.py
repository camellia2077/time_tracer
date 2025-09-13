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
        display_name = self.project_name.replace("_", " ").capitalize()
        return f"{display_name} - {year}年度活动热力图"

    def get_legend(self) -> str:
        boxes = "".join([f'<div class="legend-box" style="background-color: {c};"></div>' for c in self.color_palette])
        return f'<div class="legend"><span>少</span>{boxes}<span>多</span></div>'

class BooleanStrategy(HeatmapStrategy):
    """用于通用布尔值（0 或 1）数据的策略。"""
    # ✨ 核心修改: __init__ 现在接收数据以进行统计
    def __init__(self, data_type: str, color_config: Dict[str, str], data: Dict[datetime.date, int]):
        self.data_type = data_type.capitalize()
        self.color_true = color_config.get("color_true", "#56d364")
        self.color_false = color_config.get("color_false", "#ebedf0")

        # ✨ 新增：统计 0 和 1 的数量
        self.count_true = sum(1 for v in data.values() if v == 1)
        self.count_false = len(data) - self.count_true

    def get_color_and_tooltip(self, date: datetime.date, value: int) -> Tuple[str, str]:
        date_str = date.strftime('%Y-%m-%d')
        if value == 1:
            color = self.color_true
            tooltip = f"{self.data_type}: 是 on {date_str}"
        else:
            color = self.color_false
            tooltip = f"{self.data_type}: 否 on {date_str}"
        return color, tooltip

    def get_title(self, year: int) -> str:
        return f"{self.data_type} 状态 - {year}年度热力图"

    # ✨ 核心修改: 在图例中加入统计数量
    def get_legend(self) -> str:
        return f"""
        <div class="legend">
            <div class="legend-box" style="background-color: {self.color_false};"></div><span>否 / 无记录 ({self.count_false} 天)</span>
            <div class="legend-box" style="background-color: {self.color_true};"></div><span>是 ({self.count_true} 天)</span>
        </div>
        """