import datetime
from pathlib import Path
from typing import Dict, Any
from jinja2 import Environment, FileSystemLoader

from .heatmap_strategies import HeatmapStrategy 
# 引入新的布局引擎
from .layout_engine import AnnualLayout, MonthlyLayout

class HeatmapRenderer:
    """
    通用热力图渲染器。
    职责：协调 Jinja2 模板加载、调用布局引擎计算数据、执行文件写入。
    """
    def __init__(self, year: int, data: Dict[datetime.date, Any], strategy: HeatmapStrategy):
        self.year = year
        self.data = data
        self.strategy = strategy
        
        # 初始化 Jinja2 环境
        template_dir = Path(__file__).parent.parent / "templates"
        self.env = Environment(loader=FileSystemLoader(template_dir))

    def _render_and_save(self, template_name: str, context: Dict[str, Any], output_path: Path):
        """通用的渲染并保存逻辑"""
        template = self.env.get_template(template_name)
        html_content = template.render(**context)
        try:
            with output_path.open("w", encoding="utf-8") as f:
                f.write(html_content)
        except IOError as e:
            print(f"写入文件出错 '{output_path}': {e}")

    def save_annual_heatmap(self, output_path: Path):
        """使用 AnnualLayout 计算数据并渲染"""
        layout = AnnualLayout(self.year, self.data, self.strategy)
        context = layout.build_context()
        self._render_and_save("annual_heatmap.html", context, output_path)

    def save_monthly_heatmap(self, output_path: Path):
        """使用 MonthlyLayout 计算数据并渲染"""
        layout = MonthlyLayout(self.year, self.data, self.strategy)
        context = layout.build_context()
        self._render_and_save("monthly_heatmap.html", context, output_path)