import sqlite3
import argparse
import datetime
import calendar
import os
import sys
import json

# --- Configuration ---
# Path to the database file
DB_PATH = 'time_data.db'
# Path to the color configuration file
COLOR_CONFIG_PATH = 'heatmap_colors_config.json'
# Database connection timeout in seconds
DB_CONNECTION_TIMEOUT = 10
# Maximum recursion depth for queries to prevent infinite loops
MAX_RECURSION_DEPTH = 4

def load_color_config() -> dict:
    """
    Loads color configuration from a JSON file.

    Returns:
        A dictionary containing the color palette and special colors.
        e.g., {'palette': [...], 'over_12h_color': '#f97148'}
    """
    print(f"🎨 [Step 1/4] Loading color configuration from '{COLOR_CONFIG_PATH}'...")
    
    try:
        with open(COLOR_CONFIG_PATH, 'r', encoding='utf-8') as f:
            config = json.load(f)
    except FileNotFoundError:
        print(f"❌ Error: Color configuration file '{COLOR_CONFIG_PATH}' not found.", file=sys.stderr)
        sys.exit(1)
    except json.JSONDecodeError:
        print(f"❌ Error: Invalid format in color configuration file '{COLOR_CONFIG_PATH}'.", file=sys.stderr)
        sys.exit(1)

    # --- Load color palette ---
    palette_name = config.get("DEFAULT_COLOR_PALETTE_NAME")
    if palette_name is None:
        print(f"❌ Error: 'DEFAULT_COLOR_PALETTE_NAME' key not found in '{COLOR_CONFIG_PATH}'.", file=sys.stderr)
        sys.exit(1)
        
    print(f"  Using default color palette defined in JSON: '{palette_name}'")
    color_palette = config.get("COLOR_PALETTES", {}).get(palette_name)

    if color_palette is None:
        print(f"❌ Error: Palette '{palette_name}' not found in the configuration file.", file=sys.stderr)
        sys.exit(1)
    if not isinstance(color_palette, list) or len(color_palette) != 5:
        print(f"❌ Error: Palette '{palette_name}' must be an array of 5 color values.", file=sys.stderr)
        sys.exit(1)
    
    # --- Load special color for >12 hours ---
    print(f"  Loading special color for >12 hours...")
    over_12h_ref = config.get("OVER_12_HOURS_COLOR_REF")
    if over_12h_ref is None:
        print(f"❌ Error: 'OVER_12_HOURS_COLOR_REF' key not found in the configuration file.", file=sys.stderr)
        sys.exit(1)

    over_12h_color = config.get("SINGLE_COLORS", {}).get(over_12h_ref)
    if over_12h_color is None:
        print(f"❌ Error: Color reference '{over_12h_ref}' not found in 'SINGLE_COLORS'.", file=sys.stderr)
        sys.exit(1)
    
    print(f"  ✔️  Color configuration loaded successfully: {color_palette}, Special color: {over_12h_color}")
    
    return {"palette": color_palette, "over_12h_color": over_12h_color}

def _execute_query(cursor, project_name: str, year: int) -> list:
    """
    Executes the SQL query to fetch time records for a given project and year.
    """
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
    cursor.execute(sql_query, (project_name, str(year)))
    return cursor.fetchall()

def get_project_data_for_year(year: int, project_name: str) -> dict[datetime.date, float]:
    """
    Retrieves and processes the total time spent per day for a given project and year from the database.
    """
    print(f"🔍 [Step 2/4] Retrieving data for project '{project_name}' for the year {year}...")
    
    if not os.path.exists(DB_PATH):
        print(f"❌ Error: Database file '{DB_PATH}' not found in the current directory.", file=sys.stderr)
        sys.exit(1)
    
    print(f"  ✔️  Database file '{DB_PATH}' found.")

    project_data = {}
    try:
        with sqlite3.connect(DB_PATH, timeout=DB_CONNECTION_TIMEOUT) as conn:
            cursor = conn.cursor()
            rows = _execute_query(cursor, project_name, year)
            
            print(f"  ✔️  Query executed. Found data for {len(rows)} days for '{project_name}'.")
            if not rows:
                print(f"  ⚠️  Warning: No records found for '{project_name}' in {year}.")

            for row in rows:
                date_str, total_seconds = row
                if total_seconds is not None and total_seconds > 0:
                    current_date = datetime.datetime.strptime(date_str, '%Y%m%d').date()
                    hours = total_seconds / 3600.0
                    project_data[current_date] = hours
    except Exception as e:
        print(f"❌ An error occurred during database operation: {e}", file=sys.stderr)
        sys.exit(1)
    
    print("✅ [Step 2/4] Data retrieval complete.")
    return project_data

def get_color_for_hours(hours: float, color_palette: list, over_12h_color: str) -> str:
    """Determines the heatmap color based on the number of hours and the color palette."""
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

def _generate_svg_content(year: int, project_name: str, data: dict[datetime.date, float], color_config: dict) -> tuple[str, int, int]:
    """
    Generates the SVG content for the heatmap.
    """
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
        
        tooltip = f"{hours:.2f} hours of {project_name} on {current_date.strftime('%Y-%m-%d')}"
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

    svg_content = f"""
        <svg width="{svg_width}" height="{svg_height}">
            {"\n".join(month_labels_html)}
            {"\n".join(day_labels_html)}
            {"\n".join(rects_html)}
        </svg>
    """
    
    return svg_content, svg_width, svg_height

def generate_heatmap_html(year: int, project_name: str, data: dict[datetime.date, float], color_config: dict) -> str:
    """
    Generates the full HTML content for the heatmap by embedding the SVG.
    """
    print(f"🎨 [Step 3/4] Generating SVG and HTML structure for project '{project_name}'...")
    
    svg_content, svg_width, svg_height = _generate_svg_content(year, project_name, data, color_config)
    display_project_name = project_name.capitalize()
    
    html_template = f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>{display_project_name} Heatmap - {year}</title>
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
    <h1>{display_project_name} Heatmap - {year}</h1>
    <div class="heatmap-container">
        {svg_content}
    </div>
</body>
</html>"""
    print("✅ [Step 3/4] HTML generation complete.")
    return html_template

def _write_html_to_file(filename: str, content: str):
    """
    Writes the given content to a file.
    """
    print(f"📄 [Step 4/4] Writing HTML to file '{filename}'...")
    try:
        with open(filename, "w", encoding="utf-8") as f:
            f.write(content)
        print("✅ [Step 4/4] File writing complete.")
    except IOError as e:
        print(f"❌ Error writing to file '{filename}': {e}", file=sys.stderr)
        sys.exit(1)

def main():
    """Main function to parse arguments and generate the heatmap."""
    parser = argparse.ArgumentParser(
        description="Generate a GitHub-style heatmap for a specific project from a time_data.db database.",
        formatter_class=argparse.RawTextHelpFormatter
    )
    parser.add_argument("year", type=int, help="The year to generate the heatmap for (e.g., 2024).")
    parser.add_argument("-p", "--project", type=str, default="study", help='The parent project to generate the heatmap for (e.g., "code").\nDefaults to "study".')
    args = parser.parse_args()
    
    year = args.year
    project_name = args.project.lower()

    try:
        # 1. Load color configuration
        color_config = load_color_config()
        
        # 2. Get data from the database
        project_data = get_project_data_for_year(year, project_name)

        # 3. Generate HTML content
        html_content = generate_heatmap_html(year, project_name, project_data, color_config)

        # 4. Write to file
        output_filename = f"heatmap_{project_name}_{year}.html"
        _write_html_to_file(output_filename, html_content)
        
        print(f"\n🎉 All done! Heatmap generated successfully: {output_filename}")

    except Exception as e:
        print(f"\n❌ An unexpected error occurred in the main process: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    print("🚀 Starting heatmap generator...")
    main()
