import sqlite3
import argparse
import datetime
import os
import sys
import json
from typing import Dict, Any, List

# --- Configuration ---
DB_PATH = 'time_data.db'
COLOR_CONFIG_PATH = 'heatmap_colors_config.json'
DB_CONNECTION_TIMEOUT = 10
MAX_RECURSION_DEPTH = 4

class DataReader:
    """
    负责从SQLite数据库中读取和处理时间跟踪数据。
    """
    def __init__(self, db_path: str, timeout: int, max_recursion_depth: int):
        """
        初始化DataReader。

        Args:
            db_path (str): 数据库文件的路径。
            timeout (int): 数据库连接超时时间（秒）。
            max_recursion_depth (int): 用于递归查询的最大深度。
        """
        if not os.path.exists(db_path):
            print(f"❌ 错误: 数据库文件 '{db_path}' 未找到。", file=sys.stderr)
            sys.exit(1)
            
        self.db_path = db_path
        self.timeout = timeout
        self.max_recursion_depth = max_recursion_depth
        print(f"  ✔️  DataReader 初始化成功，数据库: '{db_path}'")

    def _execute_query(self, cursor: sqlite3.Cursor, project_name: str, year: int) -> list:
        """
        执行SQL查询以获取指定项目和年份的时间记录。
        """
        sql_query = f"""
        WITH RECURSIVE target_projects(project, depth) AS (
          VALUES(?, 1) 
          UNION ALL
          SELECT pc.child, tp.depth + 1
          FROM parent_child pc JOIN target_projects tp ON pc.parent = tp.project
          WHERE tp.depth < ?
        )
        SELECT tr.date, SUM(tr.duration)
        FROM time_records tr
        WHERE tr.project_path IN (SELECT project FROM target_projects)
          AND SUBSTR(tr.date, 1, 4) = ?
        GROUP BY tr.date;
        """
        cursor.execute(sql_query, (project_name, self.max_recursion_depth, str(year)))
        return cursor.fetchall()

    def fetch_project_data(self, year: int, project_name: str) -> Dict[datetime.date, float]:
        """
        检索并处理指定项目和年份每天花费的总时间。
        """
        print(f"🔍 [步骤 2/4] 正在为项目 '{project_name}' 检索 {year} 年的数据...")
        project_data = {}
        try:
            with sqlite3.connect(self.db_path, timeout=self.timeout) as conn:
                cursor = conn.cursor()
                rows = self._execute_query(cursor, project_name, year)
                
                print(f"  ✔️  查询执行完毕。为 '{project_name}' 找到 {len(rows)} 天的数据。")
                if not rows:
                    print(f"  ⚠️  警告: 在 {year} 年未找到 '{project_name}' 的记录。")

                for row in rows:
                    date_str, total_seconds = row
                    if total_seconds is not None and total_seconds > 0:
                        current_date = datetime.datetime.strptime(date_str, '%Y%m%d').date()
                        hours = total_seconds / 3600.0
                        project_data[current_date] = hours
        except Exception as e:
            print(f"❌ 数据库操作期间发生错误: {e}", file=sys.stderr)
            sys.exit(1)
        
        print("✅ [步骤 2/4] 数据检索完成。")
        return project_data


class HeatmapGenerator:
    """
    根据提供的数据生成HTML和SVG格式的热力图。
    """
    def __init__(self, color_config: Dict[str, Any]):
        """
        初始化HeatmapGenerator。

        Args:
            color_config (Dict[str, Any]): 包含调色板和特殊颜色的配置字典。
        """
        self.color_palette = color_config.get('palette')
        self.over_12h_color = color_config.get('over_12h_color')
        if not self.color_palette or not self.over_12h_color:
            print("❌ 错误: 颜色配置不完整。", file=sys.stderr)
            sys.exit(1)
        print("  ✔️  HeatmapGenerator 初始化成功。")

    def _get_color_for_hours(self, hours: float) -> str:
        """根据小时数和调色板确定热力图的颜色。"""
        if hours > 12:
            return self.over_12h_color
        elif hours > 10:  # 10 < hours <= 12
            return self.color_palette[4]
        elif hours > 8:   # 8 < hours <= 10
            return self.color_palette[3]
        elif hours > 4:   # 4 < hours <= 8
            return self.color_palette[2]
        elif hours > 0:   # 0 < hours <= 4
            return self.color_palette[1]
        else:             # hours <= 0
            return self.color_palette[0]

    def _generate_svg_content(self, year: int, project_name: str, data: Dict[datetime.date, float]) -> str:
        """为热力图生成SVG内容。"""
        SQUARE_SIZE, SQUARE_GAP, SQUARE_RADIUS = 12, 3, 2
        GRID_UNIT = SQUARE_SIZE + SQUARE_GAP
        LEFT_PADDING, TOP_PADDING = 30, 30

        start_date = datetime.date(year, 1, 1)
        total_days = (datetime.date(year, 12, 31) - start_date).days + 1
        
        first_day_weekday = (start_date.isoweekday()) % 7
        num_weeks = (total_days + first_day_weekday + 6) // 7

        svg_width = num_weeks * GRID_UNIT + LEFT_PADDING
        svg_height = 7 * GRID_UNIT + TOP_PADDING

        rects_html, month_labels_html = [], []
        last_month = -1

        for day_index in range(total_days):
            current_date = start_date + datetime.timedelta(days=day_index)
            grid_day_index = day_index + first_day_weekday
            week_x, day_y = grid_day_index // 7, grid_day_index % 7
            x_pos, y_pos = week_x * GRID_UNIT + LEFT_PADDING, day_y * GRID_UNIT + TOP_PADDING
            
            hours = data.get(current_date, 0)
            color = self._get_color_for_hours(hours)
            
            tooltip = f"{hours:.2f} 小时 {project_name} on {current_date.strftime('%Y-%m-%d')}"
            rects_html.append(
                f'    <rect width="{SQUARE_SIZE}" height="{SQUARE_SIZE}" x="{x_pos}" y="{y_pos}" '
                f'fill="{color}" rx="{SQUARE_RADIUS}" ry="{SQUARE_RADIUS}">'
                f'<title>{tooltip}</title></rect>'
            )

            if current_date.month != last_month and current_date.weekday() < 3:
                month_labels_html.append(
                    f'  <text x="{x_pos}" y="{TOP_PADDING - 10}" class="month">{current_date.strftime("%b")}</text>'
                )
                last_month = current_date.month

        day_labels_html = [
            f'<text x="{LEFT_PADDING - 10}" y="{TOP_PADDING + GRID_UNIT * 1 + SQUARE_SIZE / 1.5}" class="day-label">M</text>',
            f'<text x="{LEFT_PADDING - 10}" y="{TOP_PADDING + GRID_UNIT * 3 + SQUARE_SIZE / 1.5}" class="day-label">W</text>',
            f'<text x="{LEFT_PADDING - 10}" y="{TOP_PADDING + GRID_UNIT * 5 + SQUARE_SIZE / 1.5}" class="day-label">F</text>'
        ]

        return f"""
        <svg width="{svg_width}" height="{svg_height}">
            {"\n".join(month_labels_html)}
            {"\n".join(day_labels_html)}
            {"\n".join(rects_html)}
        </svg>
        """

    def generate_html(self, year: int, project_name: str, data: Dict[datetime.date, float]) -> str:
        """通过嵌入SVG生成热力图的完整HTML内容。"""
        print(f"🎨 [步骤 3/4] 正在为项目 '{project_name}' 生成SVG和HTML结构...")
        
        svg_content = self._generate_svg_content(year, project_name, data)
        display_project_name = project_name.capitalize()
        
        html_template = f"""
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>{display_project_name} 热力图 - {year}</title>
    <style>
        body {{ font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif; background-color: #f6f8fa; color: #24292e; display: flex; justify-content: center; align-items: center; flex-direction: column; margin: 2em; }}
        .heatmap-container {{ border: 1px solid #e1e4e8; border-radius: 6px; padding: 20px; background-color: #ffffff; max-width: 100%; overflow-x: auto; }}
        svg {{ display: block; margin: 0 auto; }}
        .month, .day-label {{ font-size: 10px; fill: #586069; text-anchor: middle; }}
        .day-label {{ text-anchor: end; }}
        h1 {{ font-weight: 400; text-align: center; }}
        rect:hover {{ stroke: #24292e; stroke-width: 1px; }}
    </style>
</head>
<body>
    <h1>{display_project_name} 热力图 - {year}</h1>
    <div class="heatmap-container">
        {svg_content}
    </div>
</body>
</html>"""
        print("✅ [步骤 3/4] HTML生成完成。")
        return html_template

def load_color_config(config_path: str) -> Dict[str, Any]:
    """从JSON文件加载颜色配置。"""
    print(f"🎨 [步骤 1/4] 正在从 '{config_path}' 加载颜色配置...")
    try:
        with open(config_path, 'r', encoding='utf-8') as f:
            config = json.load(f)
    except FileNotFoundError:
        print(f"❌ 错误: 颜色配置文件 '{config_path}' 未找到。", file=sys.stderr)
        sys.exit(1)
    except json.JSONDecodeError:
        print(f"❌ 错误: 颜色配置文件 '{config_path}' 格式无效。", file=sys.stderr)
        sys.exit(1)

    palette_name = config.get("DEFAULT_COLOR_PALETTE_NAME")
    color_palette = config.get("COLOR_PALETTES", {}).get(palette_name)
    over_12h_ref = config.get("OVER_12_HOURS_COLOR_REF")
    over_12h_color = config.get("SINGLE_COLORS", {}).get(over_12h_ref)

    if not all([palette_name, color_palette, over_12h_ref, over_12h_color]):
        print("❌ 错误: 颜色配置文件中的键缺失或无效。", file=sys.stderr)
        sys.exit(1)
    
    print("✅ [步骤 1/4] 颜色配置加载成功。")
    return {"palette": color_palette, "over_12h_color": over_12h_color}

def write_html_to_file(filename: str, content: str):
    """将给定的内容写入文件。"""
    print(f"📄 [步骤 4/4] 正在将HTML写入文件 '{filename}'...")
    try:
        with open(filename, "w", encoding="utf-8") as f:
            f.write(content)
        print("✅ [步骤 4/4] 文件写入完成。")
    except IOError as e:
        print(f"❌ 写入文件 '{filename}' 时出错: {e}", file=sys.stderr)
        sys.exit(1)

def main():
    """主函数，用于解析参数并生成热力图。"""
    parser = argparse.ArgumentParser(
        description="从 time_data.db 数据库为指定项目生成一个GitHub风格的热力图。",
        formatter_class=argparse.RawTextHelpFormatter
    )
    parser.add_argument("year", type=int, help="要生成热力图的年份 (例如: 2024)。")
    parser.add_argument("-p", "--project", type=str, default="study", help='要生成热力图的父项目 (例如 "code")。\n默认为 "study"。')
    args = parser.parse_args()
    
    year = args.year
    project_name = args.project.lower()

    print("🚀 启动热力图生成器...")
    try:
        # 1. 加载颜色配置
        color_config = load_color_config(COLOR_CONFIG_PATH)
        
        # 2. 从数据库获取数据
        reader = DataReader(DB_PATH, DB_CONNECTION_TIMEOUT, MAX_RECURSION_DEPTH)
        project_data = reader.fetch_project_data(year, project_name)

        # 3. 生成HTML内容
        generator = HeatmapGenerator(color_config)
        html_content = generator.generate_html(year, project_name, project_data)

        # 4. 写入文件
        output_filename = f"heatmap_{project_name}_{year}.html"
        write_html_to_file(output_filename, html_content)
        
        print(f"\n🎉 全部完成！热力图已成功生成: {output_filename}")

    except Exception as e:
        print(f"\n❌ 主进程中发生意外错误: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main()
