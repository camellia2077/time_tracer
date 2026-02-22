import datetime
import calendar
from typing import Dict, Any, List
from .heatmap_strategies import HeatmapStrategy

class BaseLayout:
    """布局基类，存放共有逻辑或配置"""
    def __init__(self, year: int, data: Dict[datetime.date, Any], strategy: HeatmapStrategy):
        self.year = year
        self.data = data
        self.strategy = strategy
        self.cal = calendar.Calendar()

class AnnualLayout(BaseLayout):
    """负责年度视图的布局计算"""
    SQUARE_SIZE, SQUARE_GAP = 12, 3
    LEFT_PADDING, TOP_PADDING = 30, 30

    def build_context(self) -> Dict[str, Any]:
        GRID_UNIT = self.SQUARE_SIZE + self.SQUARE_GAP
        start_date = datetime.date(self.year, 1, 1)
        end_date = datetime.date(self.year, 12, 31)
        total_days = (end_date - start_date).days + 1
        first_day_weekday = start_date.isoweekday() % 7 
        
        num_weeks = (total_days + first_day_weekday + 6) // 7
        svg_width = num_weeks * GRID_UNIT + self.LEFT_PADDING
        svg_height = 7 * GRID_UNIT + self.TOP_PADDING

        rects, month_labels, day_labels = [], [], []
        
        # 星期标签
        for i, label in zip([1, 3, 5], ["Mon", "Wed", "Fri"]):
             y_pos = self.TOP_PADDING + GRID_UNIT * i + self.SQUARE_SIZE / 1.5
             day_labels.append({'x': self.LEFT_PADDING - 15, 'y': y_pos, 'text': label})

        last_month = -1
        for day_index in range(total_days):
            current_date = start_date + datetime.timedelta(days=day_index)
            value = self.data.get(current_date)
            
            grid_day_index = day_index + first_day_weekday
            week_x, day_y = divmod(grid_day_index, 7)
            
            x_pos = week_x * GRID_UNIT + self.LEFT_PADDING
            y_pos = day_y * GRID_UNIT + self.TOP_PADDING
            
            color, tooltip = self.strategy.get_color_and_tooltip(current_date, value)
            
            rects.append({
                'width': self.SQUARE_SIZE, 'height': self.SQUARE_SIZE,
                'x': x_pos, 'y': y_pos,
                'fill': color, 'tooltip': tooltip
            })
            
            if current_date.month != last_month:
                month_labels.append({
                    'x': x_pos, 'y': self.TOP_PADDING - 10, 
                    'text': current_date.strftime("%b")
                })
                last_month = current_date.month

        return {
            "title": self.strategy.get_title(self.year),
            "svg_width": svg_width, "svg_height": svg_height,
            "month_labels": month_labels, "day_labels": day_labels,
            "rects": rects, "legend": self.strategy.get_legend()
        }

class MonthlyLayout(BaseLayout):
    """负责月度视图的布局计算"""
    SQUARE_SIZE, SQUARE_GAP = 15, 4
    LEFT_PADDING, TOP_PADDING = 15, 40
    HEADER_HEIGHT = 30

    def build_context(self) -> Dict[str, Any]:
        months_data = []
        GRID_UNIT = self.SQUARE_SIZE + self.SQUARE_GAP
        
        for month in range(1, 13):
            month_weeks = self.cal.monthdatescalendar(self.year, month)
            svg_width = 7 * GRID_UNIT + self.LEFT_PADDING * 2
            svg_height = len(month_weeks) * GRID_UNIT + self.TOP_PADDING + self.HEADER_HEIGHT
            
            day_labels = []
            for i, day_name in enumerate(["S", "M", "T", "W", "T", "F", "S"]):
                x_pos = i * GRID_UNIT + self.LEFT_PADDING + self.SQUARE_SIZE / 2
                day_labels.append({
                    'x': x_pos, 'y': self.TOP_PADDING - 10, 'text': day_name
                })
            
            rects = []
            for week_index, week in enumerate(month_weeks):
                for day_index, current_date in enumerate(week):
                    if current_date.month != month: continue
                    
                    value = self.data.get(current_date)
                    x_pos = day_index * GRID_UNIT + self.LEFT_PADDING
                    y_pos = week_index * GRID_UNIT + self.TOP_PADDING
                    
                    color, tooltip = self.strategy.get_color_and_tooltip(current_date, value)
                    rects.append({
                        'width': self.SQUARE_SIZE, 'height': self.SQUARE_SIZE,
                        'x': x_pos, 'y': y_pos,
                        'fill': color, 'tooltip': tooltip
                    })
            
            months_data.append({
                'name': datetime.date(self.year, month, 1).strftime('%B'),
                'svg_width': svg_width, 'svg_height': svg_height,
                'day_labels': day_labels, 'rects': rects
            })

        return {
            "title": self.strategy.get_title(self.year),
            "months": months_data, "legend": self.strategy.get_legend()
        }