# db_access.py
# This module serves as a centralized Data Access Layer (DAL) for all database interactions.

import sqlite3
import pandas as pd
import datetime
import os
import sys
from typing import Dict, Tuple, Optional

# --- Configuration ---
DB_PATH = 'time_data.db'
DB_CONNECTION_TIMEOUT = 10
MAX_RECURSION_DEPTH = 4 # Used for the heatmap query

# --- Terminal Colors ---
COLOR_GREEN = '\033[92m'
COLOR_RED = '\033[91m'
COLOR_RESET = '\033[0m'

# --- Connection Management ---

def _get_db_connection():
    """Establishes and returns a database connection."""
    if not os.path.exists(DB_PATH):
        print(f"{COLOR_RED}Error: Database file not found at '{DB_PATH}'{COLOR_RESET}")
        sys.exit(1)
    try:
        conn = sqlite3.connect(DB_PATH, timeout=DB_CONNECTION_TIMEOUT)
        return conn
    except sqlite3.Error as e:
        print(f"{COLOR_RED}Database connection error: {e}{COLOR_RESET}")
        sys.exit(1)

# --- Public Data Access Functions ---

def get_data_for_timeline() -> Optional[Tuple[pd.DataFrame, pd.DataFrame, pd.DataFrame]]:
    """
    Fetches all necessary tables ('days', 'time_records', 'parent_child') for timeline generation.
    This function is intended for use by the timeline_generator.py script.

    Returns:
        A tuple containing three pandas DataFrames: (df_days, df_records, df_parents).
        Returns None if a database error occurs.
    """
    print(f"DataAccess: Fetching all tables for timeline generation from '{DB_PATH}'...")
    conn = _get_db_connection()
    try:
        df_days = pd.read_sql_query("SELECT * FROM days", conn)
        df_records = pd.read_sql_query("SELECT * FROM time_records", conn)
        df_parents = pd.read_sql_query("SELECT * FROM parent_child", conn)
        print("DataAccess: Successfully loaded all tables.")
        return df_days, df_records, df_parents
    except pd.io.sql.DatabaseError as e:
        print(f"{COLOR_RED}Error reading from database: {e}{COLOR_RESET}")
        print(f"{COLOR_RED}Please ensure the database contains 'days', 'time_records', and 'parent_child' tables.{COLOR_RESET}")
        return None
    finally:
        if conn:
            conn.close()

def get_data_for_heatmap(year: int, project_name: str) -> Dict[datetime.date, float]:
    """
    Fetches and processes the total time spent per day for a specific project and its children.
    This function is intended for use by the heatmap_generator.py script.

    Args:
        year (int): The target year to query.
        project_name (str): The name of the root project to aggregate data for.

    Returns:
        A dictionary mapping each date to the total hours spent, e.g., {datetime.date(2024, 1, 15): 2.5}.
    """
    print(f"DataAccess: Fetching heatmap data for project '{project_name}' in year {year}...")
    project_data = {}
    conn = _get_db_connection()
    try:
        cursor = conn.cursor()
        
        # This recursive query finds all child projects and sums up their time records.
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
        cursor.execute(sql_query, (project_name, MAX_RECURSION_DEPTH, str(year)))
        rows = cursor.fetchall()

        print(f"DataAccess: Found {len(rows)} days with records for '{project_name}'.")
        if not rows:
            print(f"Warning: No records found for '{project_name}' in {year}.")

        for row in rows:
            date_str, total_seconds = row
            if total_seconds is not None and total_seconds > 0:
                current_date = datetime.datetime.strptime(date_str, '%Y%m%d').date()
                hours = total_seconds / 3600.0
                project_data[current_date] = hours
                
    except sqlite3.Error as e:
        print(f"{COLOR_RED}An error occurred during database operation: {e}{COLOR_RESET}", file=sys.stderr)
        # In a real app, you might want to raise the exception instead of exiting.
        sys.exit(1)
    finally:
        if conn:
            conn.close()
            
    print("DataAccess: Heatmap data processing complete.")
    return project_data