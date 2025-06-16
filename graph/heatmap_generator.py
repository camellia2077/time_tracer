import sqlite3
import argparse
import datetime
import calendar
import os
import sys
import json

# --- 配置 ---
# 数据库文件路径
DB_PATH = 'time_data.db'
# 颜色配置文件路径
COLOR_CONFIG_PATH = 'heatmap_colors_config.json'
# 数据库连接超时时间 (秒)
DB_CONNECTION_TIMEOUT = 10
# 递归查询的最大深度，防止无限循环
MAX_RECURSION_DEPTH = 2

def load_color_config() -> dict:
    """
    从JSON配置文件加载颜色配置。

    Returns:
        一个包含调色板和特殊颜色的字典。
        e.g., {'palette': [...], 'over_12h_color': '#f97148'}
    """
    print(f"🎨 [步骤 1/4] 正在从 '{COLOR_CONFIG_PATH}' 加载颜色配置...")
    
    try:
        with open(COLOR_CONFIG_PATH, 'r', encoding='utf-8') as f:
            config = json.load(f)
    except FileNotFoundError:
        print(f"❌ 错误: 颜色配置文件 '{COLOR_CONFIG_PATH}' 未找到。", file=sys.stderr)
        sys.exit(1)
    except json.JSONDecodeError:
        print(f"❌ 错误: 颜色配置文件 '{COLOR_CONFIG_PATH}' 格式无效。", file=sys.stderr)
        sys.exit(1)

    # --- 加载调色板 ---
    palette_name = config.get("DEFAULT_COLOR_PALETTE_NAME")
    if palette_name is None:
        print(f"❌ 错误: 在配置文件 '{COLOR_CONFIG_PATH}' 中未找到 'DEFAULT_COLOR_PALETTE_NAME' 键。", file=sys.stderr)
        sys.exit(1)
        
    print(f"  将使用JSON中定义的默认调色板: '{palette_name}'")
    color_palette = config.get("COLOR_PALETTES", {}).get(palette_name)

    if color_palette is None:
        print(f"❌ 错误: 在配置文件中未找到名为 '{palette_name}' 的调色板。", file=sys.stderr)
        sys.exit(1)
    if not isinstance(color_palette, list) or len(color_palette) != 5:
        print(f"❌ 错误: 调色板 '{palette_name}' 必须是一个包含5个颜色值的数组。", file=sys.stderr)
        sys.exit(1)
    
    # --- 加载 >12 小时的特殊颜色 ---
    print(f"  正在加载 >12 小时的特殊颜色...")
    over_12h_ref = config.get("OVER_12_HOURS_COLOR_REF")
    if over_12h_ref is None:
        print(f"❌ 错误: 在配置文件中未找到 'OVER_12_HOURS_COLOR_REF' 键。", file=sys.stderr)
        sys.exit(1)

    over_12h_color = config.get("SINGLE_COLORS", {}).get(over_12h_ref)
    if over_12h_color is None:
        print(f"❌ 错误: 在 'SINGLE_COLORS' 中未找到名为 '{over_12h_ref}' 的颜色引用。", file=sys.stderr)
        sys.exit(1)
    
    print(f"  ✔️  颜色配置加载成功: {color_palette}, 特殊颜色: {over_12h_color}")
    
    return {"palette": color_palette, "over_12h_color": over_12h_color}

def get_project_data_for_year(year: int, project_name: str) -> dict[datetime.date, float]:
    """
    根据年份和项目名称，查询数据库中每天的总时长。
    """
    print(f"🔍 [步骤 2/4] 开始为年份 {year} 检索项目 '{project_name}' 的数据...")
    
    if not os.path.exists(DB_PATH):
        print(f"❌ 错误: 在当前目录下未找到数据库文件 '{DB_PATH}'。", file=sys.stderr)
        sys.exit(1)
    
    print(f"  ✔️  数据库文件 '{DB_PATH}' 已找到。")

    sql_query = f"""
    WITH RECURSIVE target_projects(project, depth) AS (
      VALUES(?, 1) 
      UNION ALL
      SELECT pc.child, tp.depth + 1
      FROM parent_child pc JOIN target_projects tp ON pc.parent = tp.project
      WHERE tp.depth < {MAX_RECURSION_DEPTH}
    )
    SELECT tr.date, SUM(tr.duration)
    FROM time_records tr
    WHERE tr.project_path IN (SELECT project FROM target_projects)
      AND SUBSTR(tr.date, 1, 4) = ?
    GROUP BY tr.date;
    """

    project_data = {}
    try:
        with sqlite3.connect(DB_PATH, timeout=DB_CONNECTION_TIMEOUT) as conn:
            cursor = conn.cursor()
            cursor.execute(sql_query, (project_name, str(year)))
            rows = cursor.fetchall()
            
            print(f"  ✔️  查询执行完毕。找到 {len(rows)} 天包含 '{project_name}' 的数据。")
            if not rows:
                print(f"  ⚠️  警告: 在 {year} 年未找到 '{project_name}' 的记录。")

            for row in rows:
                date_str, total_seconds = row
                if total_seconds is not None and total_seconds > 0:
                    current_date = datetime.datetime.strptime(date_str, '%Y%m%d').date()
                    hours = total_seconds / 3600.0
                    project_data[current_date] = hours
    except Exception as e:
        print(f"❌ 数据库操作时发生错误: {e}", file=sys.stderr)
        sys.exit(1)
    
    print("✅ [步骤 2/4] 数据检索完成。")
    return project_data

def get_color_for_hours(hours: float, color_palette: list, over_12h_color: str) -> str:
    """根据小时数和调色板决定热力图的颜色。"""
    if hours > 12:
        return over_12h_color
    elif hours > 10:  # 10 < hours <= 12
        return color_palette[4]
    elif hours > 8:   # 8 < hours <= 10
        return color_palette[3]
    elif hours > 4:   # 4 < hours <= 8
        return color_palette[2]
    elif hours > 0:   # 0 < hours <= 4
        return color_palette[1]
    else:             # hours <= 0
        return color_palette[0]

def generate_heatmap_html(year: int, project_name: str, data: dict[datetime.date, float], color_config: dict) -> str:
    """为热力图生成完整的HTML内容。"""
    print(f"🎨 [步骤 3/4] 正在为项目 '{project_name}' 生成SVG和HTML结构...")
    
    color_palette = color_config['palette']
    over_12h_color = color_config['over_12h_color']
    
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
        color = get_color_for_hours(hours, color_palette, over_12h_color)
        
        tooltip = f"{hours:.2f} 小时的 {project_name} on {current_date.strftime('%Y-%m-%d')}"
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
        <svg width="{svg_width}" height="{svg_height}">
            {"\n".join(month_labels_html)}
            {"\n".join(day_labels_html)}
            {"\n".join(rects_html)}
        </svg>
    </div>
</body>
</html>"""
    print("✅ [步骤 3/4] HTML生成完成。")
    return html_template

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

    try:
        # 1. 加载颜色配置
        color_config = load_color_config()
        
        # 2. 从数据库获取数据
        project_data = get_project_data_for_year(year, project_name)

        # 3. 生成HTML内容
        html_content = generate_heatmap_html(year, project_name, project_data, color_config)

        # 4. 写入文件
        output_filename = f"heatmap_{project_name}_{year}.html"
        print(f"📄 [步骤 4/4] 正在将HTML写入文件 '{output_filename}'...")
        with open(output_filename, "w", encoding="utf-8") as f:
            f.write(html_content)
        
        print("✅ [步骤 4/4] 文件写入完成。")
        print(f"\n🎉 全部完成！已成功生成热力图: {output_filename}")

    except Exception as e:
        print(f"\n❌ 主进程中发生意外错误: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    print("🚀 启动热力图生成器...")
    main()
