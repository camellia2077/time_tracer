import sqlite3
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import argparse
from datetime import datetime, timedelta
import sys
import os

# --- Domain Logic Class ---

class LogicalDay:
    """Represents and processes all data for a single logical day."""

    def __init__(self, raw_records, getup_time_str, parent_lookup):
        """
        Initializes and processes the data for the logical day.

        Args:
            raw_records (pd.DataFrame): The raw time_records for one specific day from the DB.
            getup_time_str (str): The get-up time string (e.g., "09:48").
            parent_lookup (dict): A dictionary for finding a child's parent.
        """
        if raw_records.empty:
            raise ValueError("Cannot create a LogicalDay from empty records.")
            
        self.raw_records = raw_records.copy()
        self.getup_time_str = getup_time_str
        self.parent_lookup = parent_lookup
        self._parent_cache = {}

        # The following attributes are populated by _process()
        self.start_time = None
        self.end_time = None
        self.processed_data = None
        
        self._process()

    def _get_ultimate_parent(self, path):
        """Finds the root parent of a project path."""
        if path in self._parent_cache:
            return self._parent_cache[path]
        original_path = path
        for _ in range(10): # Limit iterations to prevent infinite loops
            parent = self.parent_lookup.get(path)
            if parent is None or parent == path:
                self._parent_cache[original_path] = path
                return path
            path = parent
        self._parent_cache[original_path] = path 
        return path

    def _process(self):
        """Internal method to perform all data processing steps."""
        # 1. Determine the base date and calculate the logical start time
        base_date_str = self.raw_records['date'].iloc[0]
        self.start_time = datetime.strptime(f"{base_date_str} {self.getup_time_str}", "%Y%m%d %H:%M")

        # 2. Correct timestamps for activities that cross midnight
        self._correct_timestamps()

        # 3. Find the end of the logical day based on the 'sleep_night' record
        self._determine_logical_end()

        # 4. Filter records to the logical day's boundaries
        self._filter_records()

        # 5. Enrich the data with parent categories
        if not self.processed_data.empty:
            self.processed_data['parent'] = self.processed_data['project_path'].apply(self._get_ultimate_parent)

    def _correct_timestamps(self):
        """
        Converts time strings to datetime objects, handling overnight rollovers.
        This is a critical step that ensures activities like '01:30' are placed on the correct calendar day.
        """
        # Create initial timestamps assuming everything is on the base date
        self.raw_records['start_dt'] = self.raw_records.apply(lambda r: datetime.strptime(f"{r['date']} {r['start']}", "%Y%m%d %H:%M"), axis=1)
        self.raw_records['end_dt'] = self.raw_records.apply(lambda r: datetime.strptime(f"{r['date']} {r['end']}", "%Y%m%d %H:%M"), axis=1)

        # Detect when time goes backward (e.g., from 23:00 to 01:00), indicating a new day
        is_next_day = (self.raw_records['start_dt'] < self.raw_records['start_dt'].shift(1).fillna(pd.Timestamp.min))
        day_offset = is_next_day.cumsum()
        
        # Apply the day offset to all records after the first rollover
        self.raw_records['start_dt'] += pd.to_timedelta(day_offset, unit='D')
        self.raw_records['end_dt'] += pd.to_timedelta(day_offset, unit='D')

        # Handle single records that span midnight (e.g., 23:30-00:30)
        overnight_mask = self.raw_records['end_dt'] < self.raw_records['start_dt']
        self.raw_records.loc[overnight_mask, 'end_dt'] += timedelta(days=1)

    def _determine_logical_end(self):
        """Finds the end time of the 'sleep_night' record that concludes this logical day."""
        sleep_night_record = self.raw_records[
            (self.raw_records['project_path'] == 'sleep_night') & 
            (self.raw_records['start_dt'] >= self.start_time)
        ]
        
        if sleep_night_record.empty:
            print(f"Warning: Could not find 'sleep_night' to mark the end of the logical day.")
            print("Timeline will end at the last recorded activity.")
            self.end_time = self.raw_records['end_dt'].max()
        else:
            self.end_time = sleep_night_record['end_dt'].iloc[0]

    def _filter_records(self):
        """Filters the records to fit within the calculated logical start and end times."""
        self.processed_data = self.raw_records[
            (self.raw_records['start_dt'] >= self.start_time) &
            (self.raw_records['end_dt'] <= self.end_time)
        ].copy()


# --- Service and I/O Classes ---

class DatabaseHandler:
    """Handles all interactions with the SQLite database."""
    # (This class remains unchanged)
    def __init__(self, db_path):
        if not os.path.exists(db_path):
            print(f"Error: Database file not found at '{db_path}'")
            sys.exit(1)
        try:
            self.conn = sqlite3.connect(db_path)
        except sqlite3.Error as e:
            print(f"Database connection error: {e}")
            sys.exit(1)

    def load_all_tables(self):
        try:
            df_days = pd.read_sql_query("SELECT * FROM days", self.conn)
            df_records = pd.read_sql_query("SELECT * FROM time_records", self.conn)
            df_parents = pd.read_sql_query("SELECT * FROM parent_child", self.conn)
            return df_days, df_records, df_parents
        except pd.io.sql.DatabaseError as e:
            print(f"Error reading from database: {e}")
            print("Please ensure the database contains 'days', 'time_records', and 'parent_child' tables.")
            return None, None, None

    def close(self):
        if self.conn:
            self.conn.close()


class DataProcessor:
    """A higher-level processor that uses the database to create LogicalDay objects."""
    def __init__(self, db_handler):
        self.df_days, self.df_records, self.df_parents = db_handler.load_all_tables()
        if self.df_days is None:
            raise ConnectionError("Failed to load data from database.")
        self.parent_lookup = pd.Series(self.df_parents.parent.values, index=self.df_parents.child).to_dict()

    def create_logical_day(self, target_date_str):
        """Fetches data for a specific date and constructs a LogicalDay object."""
        day_info = self.df_days[self.df_days['date'] == target_date_str]
        if day_info.empty:
            print(f"Error: No data found for date {target_date_str}.")
            return None
        
        getup_time_str = day_info['getup_time'].iloc[0]
        raw_records = self.df_records[self.df_records['date'] == target_date_str]
        
        if raw_records.empty:
            print(f"Error: No time records found for {target_date_str}.")
            return None
            
        return LogicalDay(raw_records, getup_time_str, self.parent_lookup)


class TimelinePlotter:
    """Creates a timeline visualization from processed data and saves it to a file."""
    # (This class is modified to save instead of show)
    def __init__(self, logical_day):
        if not isinstance(logical_day, LogicalDay) or logical_day.processed_data.empty:
            raise ValueError("TimelinePlotter must be initialized with a valid and processed LogicalDay object.")
        self.data = logical_day.processed_data
        self.start_dt = logical_day.start_time
        self.end_dt = logical_day.end_time
        
    def save_chart(self, output_path, title):
        """Generates the timeline plot and saves it to the specified path."""
        parent_categories = sorted(self.data['parent'].unique())
        y_labels = {cat: i for i, cat in enumerate(parent_categories)}
        
        colors = plt.get_cmap('tab20', len(parent_categories))
        cat_colors = {cat: colors(i) for i, cat in enumerate(parent_categories)}

        fig, ax = plt.subplots(figsize=(15, 8))

        for _, row in self.data.iterrows():
            y_pos = y_labels[row['parent']]
            start = mdates.date2num(row['start_dt'])
            end = mdates.date2num(row['end_dt'])
            duration = end - start
            ax.barh(y_pos, duration, left=start, height=0.6, 
                    color=cat_colors[row['parent']], edgecolor='black', linewidth=0.5)

        ax.set_yticks(list(y_labels.values()))
        ax.set_yticklabels(list(y_labels.keys()))
        ax.invert_yaxis()
        ax.xaxis.set_major_formatter(mdates.DateFormatter('%H:%M'))
        ax.xaxis.set_major_locator(mdates.HourLocator(interval=1))
        ax.set_xlim(self.start_dt, self.end_dt)
        plt.setp(ax.get_xticklabels(), rotation=45, ha="right")
        ax.grid(axis='x', linestyle='--', alpha=0.6)
        ax.set_title(title, fontsize=16)
        ax.set_xlabel("Time")
        ax.set_ylabel("Activity Category")
        
        try:
            # Use bbox_inches='tight' to prevent labels from being cut off
            fig.savefig(output_path, bbox_inches='tight')
            print(f"Timeline chart successfully saved to '{output_path}'")
        except Exception as e:
            print(f"Error saving chart to '{output_path}': {e}")
        finally:
            # Close the figure to free up memory
            plt.close(fig)


# --- Main Application Runner ---

class Application:
    """Orchestrates the program flow."""
    def __init__(self, date_str):
        self.date_str = date_str
        self.db_path = 'time_data.db'

    def run(self):
        """Executes the main logic of the application."""
        db_handler = DatabaseHandler(self.db_path)
        try:
            processor = DataProcessor(db_handler)
            logical_day = processor.create_logical_day(self.date_str)

            if logical_day and logical_day.processed_data is not None:
                plotter = TimelinePlotter(logical_day)
                output_filename = f"timeline_{self.date_str}.png"
                formatted_date = datetime.strptime(self.date_str, "%Y%m%d").strftime('%B %d, %Y')
                title = f"Logical Day Timeline for {formatted_date}"
                
                plotter.save_chart(output_filename, title)

        except (ValueError, ConnectionError) as e:
            print(f"An application error occurred: {e}")
        finally:
            db_handler.close()


def main():
    """Main function to parse arguments and run the program."""
    parser = argparse.ArgumentParser(
        description='Generate a timeline visualization for a "logical day" from the time_data.db database.'
    )
    parser.add_argument(
        'date', 
        type=str, 
        help='The target date to query in YYYYMMDD format (e.g., 20250528).'
    )
    args = parser.parse_args()

    try:
        datetime.strptime(args.date, "%Y%m%d")
    except ValueError:
        print("Error: Date must be in YYYYMMDD format.")
        sys.exit(1)

    app = Application(args.date)
    app.run()

if __name__ == "__main__":
    main()