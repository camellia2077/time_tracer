import sys
import sqlite3
import json
from datetime import datetime, timedelta

class HeatmapGenerator:
    """
    Generates an HTML/SVG study heatmap for a given year based on time tracking data.
    """

    def __init__(self, conn, year, config):
        """
        Initializes the HeatmapGenerator.

        Args:
            conn (sqlite3.Connection): The database connection object.
            year (int): The year for which to generate the heatmap.
            config (dict): Configuration dictionary loaded from JSON.
        """
        self.conn = conn
        self.year = year
        self.config = config
        self.color_palettes = config['COLOR_PALETTES']
        self.single_colors = config.get('SINGLE_COLORS', {}) # Get single colors, default to empty dict if not present
        
        self.default_color_palette = self.color_palettes[config['DEFAULT_COLOR_PALETTE_NAME']]
        
        # Resolve the over 12 hours color using the reference
        over_12_hours_color_ref = config.get('OVER_12_HOURS_COLOR_REF')
        if over_12_hours_color_ref and over_12_hours_color_ref in self.single_colors:
            self.over_12_hours_color = self.single_colors[over_12_hours_color_ref]
        else:
            # Fallback to a default or raise an error if the reference is not found
            print(f"Warning: Color reference '{over_12_hours_color_ref}' not found in SINGLE_COLORS. Using default orange.")
            self.over_12_hours_color = "#f97148" # A fallback color

        self.study_times = self._fetch_study_times()
        self.heatmap_data = []
        self.svg_params = {}

    @staticmethod
    def _time_format_duration(seconds, avg_days=1):
        """
        Formats duration in seconds to a string (e.g., XhYYm) and optionally an average.
        """
        if seconds is None:
            seconds = 0
        
        total_hours = int(seconds // 3600)
        total_minutes = int((seconds % 3600) // 60)
        time_str = f"{total_hours}h{total_minutes:02d}" if total_hours > 0 else f"{total_minutes}m"
        if total_hours == 0 and total_minutes == 0:
            time_str = "0m"

        if avg_days > 1:
            avg_seconds_per_day = seconds / avg_days
            avg_hours = int(avg_seconds_per_day // 3600)
            avg_minutes = int((avg_seconds_per_day % 3600) // 60)
            avg_str = f"{avg_hours}h{avg_minutes:02d}m"
            return f"{time_str} ({avg_str}/day)"
        else:
            return time_str

    def _fetch_study_times(self):
        """
        Fetches daily total study times for the given year from the database.
        """
        cursor = self.conn.cursor()
        start_date_str = f"{self.year}0101"
        end_date_str = f"{self.year}1231"
        cursor.execute('''
            SELECT date, SUM(duration)
            FROM time_records
            WHERE date BETWEEN ? AND ?
            AND (project_path = 'study' OR project_path LIKE 'study_%')
            GROUP BY date
        ''', (start_date_str, end_date_str))
        return dict(cursor.fetchall())

    def _get_color_for_study_time(self, study_time_seconds):
        """
        Determines the heatmap cell color based on study duration.
        """
        hours = study_time_seconds / 3600
        if hours == 0:
            return self.default_color_palette[0]
        elif hours < 4:
            return self.default_color_palette[1]
        elif hours < 8:
            return self.default_color_palette[2]
        elif hours < 10:
            return self.default_color_palette[3]
        elif hours < 12:
            return self.default_color_palette[4]
        else:
            return self.over_12_hours_color

    def _prepare_heatmap_layout_data(self):
        """
        Calculates date-related layout parameters (e.g., empty days at start/end)
        and populates self.heatmap_data with tuples of (date_obj, color, study_time_seconds).
        """
        start_date_obj = datetime(self.year, 1, 1)
        end_date_obj = datetime(self.year, 12, 31)
        
        self.svg_params['start_date_obj'] = start_date_obj
        self.svg_params['end_date_obj'] = end_date_obj

        # Sunday = 0, Monday = 1, ..., Saturday = 6 for column calculation
        # Python's isoweekday(): Mon=1..Sun=7. So (isoweekday % 7) gives Sun=0, Mon=1..Sat=6
        front_empty_days = start_date_obj.isoweekday() % 7
        self.svg_params['front_empty_days'] = front_empty_days

        total_days_in_year = (end_date_obj - start_date_obj).days + 1
        
        total_slots = front_empty_days + total_days_in_year
        back_empty_days = (7 - (total_slots % 7)) % 7
        
        self.heatmap_data = []
        for _ in range(front_empty_days):
            self.heatmap_data.append((None, 'empty', 0)) # placeholder for empty cell

        current_date = start_date_obj
        while current_date <= end_date_obj:
            date_str_yyyymmdd = current_date.strftime("%Y%m%d")
            study_time_seconds = self.study_times.get(date_str_yyyymmdd, 0)
            color = self._get_color_for_study_time(study_time_seconds)
            self.heatmap_data.append((current_date, color, study_time_seconds))
            current_date += timedelta(days=1)

        for _ in range(back_empty_days):
            self.heatmap_data.append((None, 'empty', 0)) # placeholder for empty cell
        
        self.svg_params['total_days_in_year'] = total_days_in_year # For clarity, though not directly used in SVG dims

    def _calculate_svg_dimensions(self):
        """
        Calculates SVG dimensions, cell sizes, margins, and other rendering parameters.
        """
        if not self.heatmap_data:
            self._prepare_heatmap_layout_data()

        self.svg_params['cell_size'] = 12
        self.svg_params['spacing'] = 3
        self.svg_params['weeks'] = len(self.heatmap_data) // 7
        self.svg_params['rows'] = 7

        self.svg_params['margin_top'] = 30
        self.svg_params['margin_left'] = 35
        
        self.svg_params['width'] = (
            self.svg_params['margin_left'] + 
            self.svg_params['weeks'] * (self.svg_params['cell_size'] + self.svg_params['spacing']) - 
            self.svg_params['spacing']
        )
        self.svg_params['height'] = (
            self.svg_params['margin_top'] + 
            self.svg_params['rows'] * (self.svg_params['cell_size'] + self.svg_params['spacing']) - 
            self.svg_params['spacing']
        )

    def _generate_svg_header(self):
        """Generates the opening SVG tag."""
        return f'<svg xmlns="http://www.w3.org/2000/svg" width="{self.svg_params["width"]}" height="{self.svg_params["height"]}" style="font-family: Arial, sans-serif;">'

    def _generate_day_labels_svg(self):
        """Generates SVG for day labels (Mon, Wed, Fri)."""
        svg_elements = []
        day_labels_display = {1: 'Mon', 3: 'Wed', 5: 'Fri'} 
        for i in range(self.svg_params['rows']):
            if i in day_labels_display:
                y_pos = (
                    self.svg_params['margin_top'] + 
                    i * (self.svg_params['cell_size'] + self.svg_params['spacing']) + 
                    self.svg_params['cell_size'] - 2
                )
                svg_elements.append(f'<text x="0" y="{y_pos}" font-size="10px" fill="#767676" alignment-baseline="middle">{day_labels_display[i]}</text>')
        return svg_elements

    def _generate_month_labels_svg(self):
        """Generates SVG for month labels."""
        svg_elements = []
        month_names = ["Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"]
        last_month_drawn = -1
        
        for week_idx in range(self.svg_params['weeks']):
            actual_date_in_col = None
            for day_in_week_idx in range(self.svg_params['rows']):
                heatmap_data_idx = week_idx * self.svg_params['rows'] + day_in_week_idx
                if heatmap_data_idx < len(self.heatmap_data) and self.heatmap_data[heatmap_data_idx][0] is not None:
                    actual_date_in_col = self.heatmap_data[heatmap_data_idx][0]
                    break
            
            if actual_date_in_col:
                current_month_idx = actual_date_in_col.month - 1
                if current_month_idx != last_month_drawn:
                    if actual_date_in_col.day < 8 or week_idx == 0 :
                        x_pos = (
                            self.svg_params['margin_left'] + 
                            week_idx * (self.svg_params['cell_size'] + self.svg_params['spacing'])
                        )
                        svg_elements.append(f'<text x="{x_pos}" y="{self.svg_params["margin_top"] - 10}" font-size="10px" fill="#767676">{month_names[current_month_idx]}</text>')
                        last_month_drawn = current_month_idx
        return svg_elements

    def _generate_data_cells_svg(self):
        """Generates SVG for each data cell (rectangles) in the heatmap."""
        svg_elements = []
        for i, (date_obj, color, study_time_seconds) in enumerate(self.heatmap_data):
            col_idx = i // self.svg_params['rows']
            row_idx = i % self.svg_params['rows']

            x_pos = (
                self.svg_params['margin_left'] + 
                col_idx * (self.svg_params['cell_size'] + self.svg_params['spacing'])
            )
            y_pos = (
                self.svg_params['margin_top'] + 
                row_idx * (self.svg_params['cell_size'] + self.svg_params['spacing'])
            )

            if date_obj is not None:
                duration_str = self._time_format_duration(study_time_seconds)
                title_text = f"{date_obj.strftime('%Y-%m-%d')}: {duration_str}"
                svg_elements.append(f'  <rect width="{self.svg_params["cell_size"]}" height="{self.svg_params["cell_size"]}" x="{x_pos}" y="{y_pos}" fill="{color}" rx="2" ry="2">')
                svg_elements.append(f'    <title>{title_text}</title>')
                svg_elements.append(f'  </rect>')
        return svg_elements

    def generate_html_output(self, output_filename):
        """
        Orchestrates the generation of the full heatmap SVG,
        embeds it in an HTML structure, and writes it to the specified file.
        """
        self._prepare_heatmap_layout_data()
        self._calculate_svg_dimensions()

        svg_components = []
        svg_components.append(self._generate_svg_header())
        svg_components.extend(self._generate_day_labels_svg())
        svg_components.extend(self._generate_month_labels_svg())
        svg_components.extend(self._generate_data_cells_svg())
        svg_components.append('</svg>')

        full_svg_content = '\n'.join(svg_components)

        html_content = f"""
    <!DOCTYPE html>
    <html>
    <head>
        <meta charset="UTF-8">
        <title>Study Heatmap {self.year}</title>
        <style>
            body {{ font-family: -apple-system, BlinkMacMacSystemFont, "Segoe UI", Helvetica, Arial, sans-serif, "Apple Color Emoji", "Segoe UI Emoji"; }}
            .heatmap-container {{
                display: inline-block;
                padding: 15px;
                border: 1px solid #d0d7de;
                border-radius: 6px;
                background-color: #ffffff;
            }}
             h2 {{ margin-left: {self.svg_params.get('margin_left', 35)}px; font-weight: 400; color: #24292f;}}
        </style>
    </head>
    <body>
        <div class="heatmap-container">
        <h2>Study Activity for {self.year}</h2>
        {full_svg_content}
        </div>
    </body>
    </html>
    """
        with open(output_filename, 'w', encoding='utf-8') as f:
            f.write(html_content)

def load_config(config_path='heatmap_colors_config.json'):
    """Loads the configuration from a JSON file."""
    try:
        with open(config_path, 'r', encoding='utf-8') as f:
            return json.load(f)
    except FileNotFoundError:
        print(f"Error: Configuration file '{config_path}' not found.")
        sys.exit(1)
    except json.JSONDecodeError:
        print(f"Error: Could not decode JSON from '{config_path}'. Check file format.")
        sys.exit(1)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python heatmap_generator.py <year>")
        sys.exit(1)

    try:
        year = int(sys.argv[1])
    except ValueError:
        print("Error: Year must be an integer.")
        sys.exit(1)

    config = load_config()

    db_path = 'time_data.db'
    conn = None
    try:
        conn = sqlite3.connect(db_path)
        generator = HeatmapGenerator(conn, year, config)
        output_filename = f"study_heatmap_{year}.html"
        generator.generate_html_output(output_filename)
        print(f"Heatmap generated successfully: {output_filename}")
    except sqlite3.Error as e:
        print(f"Database error: {e}")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
    finally:
        if conn:
            conn.close()