import sqlite3
import argparse
import datetime
import calendar
import os
import sys

# --- Configuration ---
# The path to your SQLite database file.
DB_PATH = 'time_data.db'
# The five colors for the heatmap, from least to most activity.
GITHUB_COLORS = ["#ebedf0", "#9be9a8", "#40c463", "#30a14e", "#216e39"]
# Database connection timeout in seconds.
DB_CONNECTION_TIMEOUT = 10
# Maximum recursion depth for the SQL query to prevent infinite loops.
MAX_RECURSION_DEPTH = 4 # 递归深度的限制

def get_study_data_for_year(year: int) -> dict[datetime.date, float]:
    """
    Queries the database for total study time per day for a given year.

    This function identifies all projects related to 'study' using a recursive
    query on the 'parent_child' table, then sums the duration for those
    projects for each day in the specified year.

    Args:
        year: The four-digit year to fetch data for.

    Returns:
        A dictionary mapping date objects to the total hours of study.
    """
    print(f"🔍 [Step 1/4] Starting data retrieval for the year {year}...")
    
    # --- File Existence Check ---
    if not os.path.exists(DB_PATH):
        print(f"❌ Error: Database file '{DB_PATH}' not found in the current directory.", file=sys.stderr)
        # Use sys.exit() to terminate immediately with an error code.
        sys.exit(1)
    
    print(f"  ✔️  Database file '{DB_PATH}' found.")

    # --- SQL Query with Recursion Safeguard ---
    # This SQL query uses a Recursive Common Table Expression (CTE) to find all
    # projects that are descendants of the main 'study' project.
    # A 'depth' column is added to prevent infinite loops from circular dependencies.
    sql_query = f"""
    WITH RECURSIVE study_projects(project, depth) AS (
      -- Base case: the 'study' project itself at depth 1
      VALUES('study', 1)
      UNION ALL
      -- Recursive step: find all children of the projects found so far
      SELECT
        pc.child,
        sp.depth + 1
      FROM
        parent_child pc JOIN study_projects sp ON pc.parent = sp.project
      WHERE
        sp.depth < {MAX_RECURSION_DEPTH} -- Safeguard to prevent infinite loops
    )
    SELECT
      tr.date,
      SUM(tr.duration) AS total_duration_seconds
    FROM
      time_records tr
    WHERE
      tr.project_path IN (SELECT project FROM study_projects)
      AND SUBSTR(tr.date, 1, 4) = ? -- Filter by the provided year
    GROUP BY
      tr.date;
    """

    study_data = {}
    try:
        print(f"  Connecting to database (timeout: {DB_CONNECTION_TIMEOUT}s)...")
        # Added a timeout to the connect call.
        with sqlite3.connect(DB_PATH, timeout=DB_CONNECTION_TIMEOUT) as conn:
            cursor = conn.cursor()
            print("  ✔️  Database connection successful.")
            print("  Executing complex SQL query to find all study records...")
            
            # Execute the query
            cursor.execute(sql_query, (str(year),))
            rows = cursor.fetchall()
            
            print(f"  ✔️  Query executed. Found {len(rows)} days with study data.")

            if not rows:
                print("  ⚠️  Warning: No study records were found for the year {year}. The heatmap will be empty.")

            for row in rows:
                date_str, total_seconds = row
                if total_seconds is not None and total_seconds > 0:
                    # Convert YYYYMMDD string to a date object
                    current_date = datetime.datetime.strptime(date_str, '%Y%m%d').date()
                    # Convert duration from seconds to hours
                    hours = total_seconds / 3600.0
                    study_data[current_date] = hours

    except sqlite3.TimeoutError:
        print(f"❌ Error: Could not connect to the database within {DB_CONNECTION_TIMEOUT} seconds. It might be locked by another program.", file=sys.stderr)
        sys.exit(1)
    except sqlite3.OperationalError as e:
        print(f"❌ SQL Error: An error occurred while querying the database: {e}", file=sys.stderr)
        print("   Please ensure 'time_data.db' is a valid SQLite database and contains the tables 'time_records' and 'parent_child' with the correct columns.", file=sys.stderr)
        sys.exit(1)
    except ValueError as e:
        print(f"❌ Data Error: A date in your database has an incorrect format. {e}", file=sys.stderr)
        sys.exit(1)
    
    print("✅ [Step 1/4] Data retrieval complete.")
    return study_data

def get_color_for_hours(hours: float) -> str:
    """
    Determines the heatmap color based on the number of study hours.
    The thresholds are based on the request: 0, 4, 8, 10 hours.
    """
    if hours <= 0:
        return GITHUB_COLORS[0]
    elif hours <= 4:
        return GITHUB_COLORS[1]
    elif hours <= 8:
        return GITHUB_COLORS[2]
    elif hours <= 10:
        return GITHUB_COLORS[3]
    else: # hours > 10
        return GITHUB_COLORS[4]

def generate_heatmap_html(year: int, data: dict[datetime.date, float]) -> str:
    """
    Generates the full HTML content for the heatmap SVG.
    """
    print("🎨 [Step 2/4] Generating SVG and HTML structure...")
    
    # --- SVG and Grid Configuration ---
    SQUARE_SIZE = 12
    SQUARE_GAP = 3
    SQUARE_RADIUS = 2 # For rounded corners
    GRID_UNIT = SQUARE_SIZE + SQUARE_GAP
    LEFT_PADDING = 30
    TOP_PADDING = 30

    start_date = datetime.date(year, 1, 1)
    end_date = datetime.date(year, 12, 31)
    total_days = (end_date - start_date).days + 1
    
    first_day_weekday = (start_date.isoweekday()) % 7
    num_weeks = (total_days + first_day_weekday + 6) // 7

    svg_width = num_weeks * GRID_UNIT + LEFT_PADDING
    svg_height = 7 * GRID_UNIT + TOP_PADDING

    rects_html = []
    month_labels_html = []
    last_month = -1

    for day_index in range(total_days):
        current_date = start_date + datetime.timedelta(days=day_index)
        grid_day_index = day_index + first_day_weekday
        week_x = grid_day_index // 7
        day_y = grid_day_index % 7

        x_pos = week_x * GRID_UNIT + LEFT_PADDING
        y_pos = day_y * GRID_UNIT + TOP_PADDING
        
        hours = data.get(current_date, 0)
        color = get_color_for_hours(hours)
        
        tooltip = f"{hours:.2f} hours of study on {current_date.strftime('%Y-%m-%d')}"
        rects_html.append(
            f'    <rect width="{SQUARE_SIZE}" height="{SQUARE_SIZE}" x="{x_pos}" y="{y_pos}" '
            f'fill="{color}" rx="{SQUARE_RADIUS}" ry="{SQUARE_RADIUS}">'
            f'<title>{tooltip}</title></rect>'
        )

        if current_date.month != last_month and current_date.weekday() < 3:
            month_name = current_date.strftime("%b")
            month_labels_html.append(
                f'  <text x="{x_pos}" y="{TOP_PADDING - 10}" class="month">{month_name}</text>'
            )
            last_month = current_date.month

    day_labels_html = [
        f'<text x="{LEFT_PADDING - 10}" y="{TOP_PADDING + GRID_UNIT * 1 + SQUARE_SIZE / 1.5}" class="day-label">M</text>',
        f'<text x="{LEFT_PADDING - 10}" y="{TOP_PADDING + GRID_UNIT * 3 + SQUARE_SIZE / 1.5}" class="day-label">W</text>',
        f'<text x="{LEFT_PADDING - 10}" y="{TOP_PADDING + GRID_UNIT * 5 + SQUARE_SIZE / 1.5}" class="day-label">F</text>'
    ]
    
    html_template = f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Study Heatmap for {year}</title>
    <style>
        body {{
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Helvetica, Arial, sans-serif, "Apple Color Emoji", "Segoe UI Emoji";
            background-color: #f6f8fa;
            color: #24292e;
            display: flex;
            justify-content: center;
            align-items: center;
            flex-direction: column;
            margin: 2em;
        }}
        .heatmap-container {{
            border: 1px solid #e1e4e8;
            border-radius: 6px;
            padding: 20px;
            background-color: #ffffff;
            max-width: 100%;
            overflow-x: auto;
        }}
        svg {{
            display: block;
            margin: 0 auto;
        }}
        .month, .day-label {{
            font-size: 10px;
            fill: #586069;
            text-anchor: middle;
        }}
        .day-label {{ text-anchor: end; }}
        h1 {{ font-weight: 400; text-align: center; }}
        rect:hover {{ stroke: #24292e; stroke-width: 1px; }}
    </style>
</head>
<body>
    <h1>Study Heatmap for {year}</h1>
    <div class="heatmap-container">
        <svg width="{svg_width}" height="{svg_height}">
            {"\n".join(month_labels_html)}
            {"\n".join(day_labels_html)}
            {"\n".join(rects_html)}
        </svg>
    </div>
</body>
</html>
"""
    print("✅ [Step 2/4] HTML generation complete.")
    return html_template

def main():
    """Main function to parse arguments and generate the heatmap."""
    parser = argparse.ArgumentParser(
        description="Generate a GitHub-style heatmap for 'study' time from the time_data.db SQLite database."
    )
    parser.add_argument(
        "year",
        type=int,
        help="The year for which to generate the heatmap (e.g., 2024)."
    )
    args = parser.parse_args()
    year = args.year

    try:
        # 1. Fetch data from the database
        study_data = get_study_data_for_year(year)

        # 2. Generate the HTML content
        html_content = generate_heatmap_html(year, study_data)

        # 3. Write the HTML to a file
        output_filename = f"study_heatmap_{year}.html"
        print(f"📄 [Step 3/4] Writing HTML to file '{output_filename}'...")
        with open(output_filename, "w", encoding="utf-8") as f:
            f.write(html_content)
        
        print(f"✅ [Step 3/4] File writing complete.")
        print(f"\n🎉 All done! Successfully generated heatmap: {output_filename}")

    except Exception as e:
        print(f"\n❌ An unexpected error occurred in the main process: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    print("🚀 Starting Heatmap Generator...")
    main()

