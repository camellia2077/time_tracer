import os
from pathlib import Path
from ..core.config import AppConfig
from ..data.sqlite_source import SQLiteSource
from ..rendering.heatmap_renderer import HeatmapRenderer
from ..rendering.heatmap_strategies import NumericStrategy, BooleanStrategy

class GeneratorService:
    """核心服务，负责编排热力图的生成流程。"""
    def __init__(self, config: AppConfig, data_source: SQLiteSource, base_dir: Path):
        self.config = config
        self.data_source = data_source
        self.base_dir = Path(base_dir) # 确保是 Path
        
        self.parent_dir = self.base_dir / "html" / "parent"
        self.bool_dir = self.base_dir / "html" / "bool"
        
        # 使用 pathlib 创建目录
        self.parent_dir.mkdir(parents=True, exist_ok=True)
        self.bool_dir.mkdir(parents=True, exist_ok=True)

    def generate_all(self):
        """生成所有配置的热力图。"""
        self._generate_numeric_heatmap()
        self._generate_boolean_heatmaps()
        print("\n所有热力图生成完毕！")

    def _generate_numeric_heatmap(self):
        """处理数值型热力图。"""
        cfg = self.config.heatmap_settings
        year = cfg.get("year", 2025)
        project_name = cfg.get("project_name")
        
        heatmap_data = self.data_source.fetch_project_duration_data(project_name, year)
        if not heatmap_data:
            print(f"⚠️ 未找到项目 '{project_name}' 在 {year} 年的数据。")
            return

        strategy_color_config = {
            'palette': self.config.color_settings["COLOR_PALETTES"][cfg["color_palette"]],
            'over_12h': self.config.color_settings["SINGLE_COLORS"][cfg["over_12_hours_color"]]
        }
        strategy = NumericStrategy(project_name, strategy_color_config)
        renderer = HeatmapRenderer(year, heatmap_data, strategy)
        
        # --- [核心修改开始] ---
        
        # 1. 获取父项目列表
        parent_projects = self.config.color_settings.get("PARENT_PROJECTS", [])
        
        # 2. 动态构建文件名：{project_name}_heatmap_annual.html
        # 例如: "study" -> "study_heatmap_annual.html"
        basename = f"{project_name}_heatmap"
        
        # 3. 决定输出目录
        # 直接判断当前项目名称是否在父项目列表中
        is_parent = project_name in parent_projects
        output_dir = self.parent_dir if is_parent else self.base_dir
        
        print(f"输出策略: 项目='{project_name}', 是否父项目={is_parent} -> 目录='{output_dir.name}'")

        # 4. 拼接完整路径
        annual_output = output_dir / f"{basename}_annual.html"
        monthly_output = output_dir / f"{basename}_monthly.html"
        
        # --- [核心修改结束] ---
        
        renderer.save_annual_heatmap(annual_output)
        print(f"✅ 年度热力图已保存: {annual_output}")
        renderer.save_monthly_heatmap(monthly_output)
        print(f"✅ 月度热力图已保存: {monthly_output}")

    def _generate_boolean_heatmaps(self):
        """处理所有布尔型热力图。"""
        cfg = self.config.bool_heatmap_settings
        year = self.config.heatmap_settings.get("year", 2025)
        reports = cfg.get("enabled_reports", [])
        
        if not reports:
            return
            
        print("\n--- 正在生成布尔型热力图，将输出到 'html/bool/' 目录 ---")
        color_cfg = self.config.color_settings.get("BOOLEAN_COLORS", {})
        
        for report_type in reports:
            bool_data = self.data_source.fetch_boolean_data(report_type, year)
            if not bool_data:
                continue
                
            strategy = BooleanStrategy(report_type, color_cfg, bool_data)
            renderer = HeatmapRenderer(year, bool_data, strategy)
            
            basename = cfg["outputs"].get(report_type, report_type)
            annual_output = self.bool_dir / f"{basename}_annual.html"
            monthly_output = self.bool_dir / f"{basename}_monthly.html"
            
            renderer.save_annual_heatmap(annual_output)
            print(f"✅ 年度热力图已保存: {annual_output}")
            renderer.save_monthly_heatmap(monthly_output)
            print(f"✅ 月度热力图已保存: {monthly_output}")