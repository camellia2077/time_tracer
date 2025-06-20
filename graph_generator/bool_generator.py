import sqlite3
import argparse
import datetime
import calendar
import os

def get_sleep_data(db_path, year):
    """
    Queries the SQLite database for sleep data for a specific year.

    Args:
        db_path (str): The path to the SQLite database file.
        year (int): The year to fetch data for.

    Returns:
        A dictionary mapping dates to sleep status ('True' or 'False').
        Returns None if the database file doesn't exist.
    """
    if not os.path.exists(db_path):
        print(f"Error: Database file not found at '{db_path}'")
        return None

    data = {}
    conn = None
    try:
        # Connect to the SQLite database
        conn = sqlite3.connect(db_path)
        cur = conn.cursor()

        # Query the 'days' table for the given year
        # The date format in the DB is 'YYYYMMDD' text
        query = "SELECT date, sleep FROM days WHERE SUBSTR(date, 1, 4) = ?"
        cur.execute(query, (str(year),))
        rows = cur.fetchall()

        for row in rows:
            date_str, sleep_status = row
            try:
                day_date = datetime.datetime.strptime(date_str, '%Y%m%d').date()
                data[day_date] = sleep_status
            except ValueError:
                print(f"Warning: Skipping invalid date format '{date_str}' in database.")

    except sqlite3.Error as e:
        print(f"Database error: {e}")
        return {}
    finally:
        if conn:
            conn.close()
    return data

def _generate_svg_content(year, sleep_data):
    """
    Generates the SVG content for the annual sleep heatmap.
    This function's style is based on heatmap_generator.py.
    """
    # Constants for SVG layout
    SQUARE_SIZE, SQUARE_GAP, SQUARE_RADIUS = 12, 3, 2
    GRID_UNIT = SQUARE_SIZE + SQUARE_GAP
    LEFT_PADDING, TOP_PADDING = 30, 30
    
    start_date = datetime.date(year, 1, 1)
    
    # Calculate total days and number of weeks to determine SVG dimensions
    total_days = (datetime.date(year, 12, 31) - start_date).days + 1
    first_day_weekday = start_date.isoweekday() % 7 # Monday is 1, Sunday is 7 -> make Sunday 0
    num_weeks = (total_days + first_day_weekday + 6) // 7

    svg_width = num_weeks * GRID_UNIT + LEFT_PADDING
    svg_height = 7 * GRID_UNIT + TOP_PADDING
    
    rects_svg = []
    month_labels_svg = []
    
    last_month = -1

    # Loop through all days of the year
    for day_index in range(total_days):
        current_date = start_date + datetime.timedelta(days=day_index)
        
        # Calculate grid position
        grid_day_index = day_index + first_day_weekday
        week_x = grid_day_index // 7
        day_y = grid_day_index % 7
        
        x_pos = week_x * GRID_UNIT + LEFT_PADDING
        y_pos = day_y * GRID_UNIT + TOP_PADDING

        # Determine color and tooltip based on sleep_data
        sleep_status = sleep_data.get(current_date, 'N/A') # Get status, default to 'N/A' for no data

        if sleep_status == 'True':
            color = '#9be9a8'  # Green for True
            status_text = "Sleep: True"
        elif sleep_status == 'False':
            color = '#e5534b'  # Red for False
            status_text = "Sleep: False"
        else: # 'N/A' (no data)
            color = '#ebedf0'  # Gray for no data
            status_text = "No Data"
        
        tooltip = f"{status_text} on {current_date.strftime('%Y-%m-%d')}"

        # Create the <rect> element with a nested <title> for the tooltip
        rects_svg.append(
            f'    <rect width="{SQUARE_SIZE}" height="{SQUARE_SIZE}" x="{x_pos}" y="{y_pos}" '
            f'fill="{color}" rx="{SQUARE_RADIUS}" ry="{SQUARE_RADIUS}">'
            f'<title>{tooltip}</title></rect>'
        )
        
        # Add month labels
        if current_date.month != last_month:
            month_labels_svg.append(
                f'  <text x="{x_pos}" y="{TOP_PADDING - 10}" class="month">{current_date.strftime("%b")}</text>'
            )
            last_month = current_date.month

    # Create day labels (Mon, Wed, Fri)
    day_labels_svg = [
        f'<text x="{LEFT_PADDING - 15}" y="{TOP_PADDING + GRID_UNIT * 1 + SQUARE_SIZE / 1.5}" class="day-label">Mon</text>',
        f'<text x="{LEFT_PADDING - 15}" y="{TOP_PADDING + GRID_UNIT * 3 + SQUARE_SIZE / 1.5}" class="day-label">Wed</text>',
        f'<text x="{LEFT_PADDING - 15}" y="{TOP_PADDING + GRID_UNIT * 5 + SQUARE_SIZE / 1.5}" class="day-label">Fri</text>'
    ]

    # Combine all parts into the final SVG string
    return f"""
<svg width="{svg_width}" height="{svg_height}">
    <g>
        {"\n".join(month_labels_svg)}
        {"\n".join(day_labels_svg)}
        {"\n".join(rects_svg)}
    </g>
</svg>
    """

def generate_heatmap_html(year, sleep_data):
    """
    Generates the HTML content for the sleep heatmap calendar using SVG.
    This version is styled to look like the output of heatmap_generator.py.
    """
    svg_content = _generate_svg_content(year, sleep_data)

    # HTML template with CSS inspired by heatmap_generator.py
    html_template = f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Sleep Status Heatmap - {year}</title>
    <style>
        body {{ font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif; background-color: #f6f8fa; color: #24292e; display: flex; flex-direction: column; align-items: center; margin: 2em; }}
        .heatmap-container {{ border: 1px solid #e1e4e8; border-radius: 6px; padding: 20px; background-color: #ffffff; }}
        svg {{ display: block; margin: 0 auto; }}
        .month, .day-label {{ font-size: 10px; fill: #586069; }}
        .day-label {{ text-anchor: middle; }}
        h1 {{ font-weight: 400; text-align: center; margin-bottom: 1em; }}
        rect:hover {{ stroke: #24292e; stroke-width: 1.5px; }}
        .legend {{ display: flex; justify-content: flex-end; align-items: center; gap: 5px; margin-top: 10px; font-size: 12px; color: #586069; }}
        .legend span {{ margin-right: 2px; }}
        .legend-box {{ width: 12px; height: 12px; border-radius: 2px; margin-right: 8px;}}
    </style>
</head>
<body>
    <h1>Sleep Status - {year}</h1>
    <div class="heatmap-container">
        {svg_content}
        <div class="legend">
            <span>No Data</span>
            <div class="legend-box" style="background-color: #ebedf0;"></div>
            <span>Sleep: True</span>
            <div class="legend-box" style="background-color: #9be9a8;"></div>
            <span>Sleep: False</span>
            <div class="legend-box" style="background-color: #e5534b;"></div>
        </div>
    </div>
</body>
</html>
    """
    return html_template

def main():
    """Main function to parse arguments and run the script."""
    parser = argparse.ArgumentParser(
        description="Generate a GitHub-style sleep heatmap for a given year from a time_data.db file.",
        formatter_class=argparse.RawTextHelpFormatter
    )
    parser.add_argument("year", type=int, help="The year for which to generate the heatmap (e.g., 2024).")
    args = parser.parse_args()
    
    year = args.year
    db_path = 'time_data.db'
    output_filename = f"{year}_sleep_heatmap.html"

    print(f"Fetching data for year {year} from '{db_path}'...")
    sleep_data = get_sleep_data(db_path, year)
    
    if sleep_data is None:
        print("Failed to fetch data. Exiting.")
        return

    print(f"Found data for {len(sleep_data)} days.")
    print(f"Generating HTML file: {output_filename}...")
    html_content = generate_heatmap_html(year, sleep_data)
    
    try:
        with open(output_filename, 'w', encoding='utf-8') as f:
            f.write(html_content)
        print(f"Successfully created '{output_filename}'.")
    except IOError as e:
        print(f"Error writing to file: {e}")

if __name__ == "__main__":
    main()