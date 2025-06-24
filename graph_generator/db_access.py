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

def _get_db_connection() -> Optional[sqlite3.Connection]:
    """
    Establishes and returns a database connection.
    Returns None if the database file cannot be found.
    """
    if not os.path.exists(DB_PATH):
        print(f"{COLOR_RED}Error: Database file not found at '{DB_PATH}'{COLOR_RESET}")
        return None
    try:
        conn = sqlite3.connect(DB_PATH, timeout=DB_CONNECTION_TIMEOUT)
        return conn
    except sqlite3.Error as e:
        print(f"{COLOR_RED}Database connection error: {e}{COLOR_RESET}")
        return None

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
    if not conn:
        return None
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
    Retrieves and processes the total time spent per day for a given project and year.
    This function includes a recursive query to sum up time from all child projects.

    Args:
        year (int): The year for which to fetch data.
        project_name (str): The parent project name.

    Returns:
        A dictionary mapping dates to the total hours recorded.
    """
    print(f"🔍 [DataAccess] Fetching heatmap data for project '{project_name}' in year {year}...")
    project_data = {}
    conn = _get_db_connection()
    if not conn:
        return {} # Return empty dict if connection fails
        
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

        print(f"  ✔️  [DataAccess] Query executed. Found {len(rows)} days with records for '{project_name}'.")
        if not rows:
            print(f"  ⚠️  [DataAccess] Warning: No records found for '{project_name}' in {year}.")

        for row in rows:
            date_str, total_seconds = row
            if total_seconds is not None and total_seconds > 0:
                current_date = datetime.datetime.strptime(date_str, '%Y%m%d').date()
                hours = total_seconds / 3600.0
                project_data[current_date] = hours
                
    except sqlite3.Error as e:
        print(f"{COLOR_RED}An error occurred during database operation: {e}{COLOR_RESET}", file=sys.stderr)
        return {} # Return empty dict on error
    finally:
        if conn:
            conn.close()
            
    print(f"✅ [DataAccess] Heatmap data processing complete for '{project_name}'.")
    return project_data

def get_sleep_data_for_bool_heatmap(year: int) -> Optional[Dict[datetime.date, str]]:
    """
    Queries the SQLite database for boolean sleep data for a specific year.
    This function is intended for use by the bool_generator.py script.

    Args:
        year (int): The year for which to fetch data.

    Returns:
        A dictionary mapping dates to sleep status ('True' or 'False'),
        or None if the database connection fails.
    """
    print(f"DataAccess: Fetching boolean sleep data for year {year}...")
    data: Dict[datetime.date, str] = {}
    conn = _get_db_connection()
    if not conn:
        return None

    try:
        cur = conn.cursor()
        query = "SELECT date, sleep FROM days WHERE SUBSTR(date, 1, 4) = ?"
        cur.execute(query, (str(year),))
        rows = cur.fetchall()

        for date_str, sleep_status in rows:
            try:
                day_date = datetime.datetime.strptime(date_str, '%Y%m%d').date()
                data[day_date] = sleep_status
            except ValueError:
                print(f"Warning: Skipping invalid date format '{date_str}' in database.")

        print(f"DataAccess: Found sleep data for {len(data)} days in {year}.")

    except sqlite3.Error as e:
        print(f"{COLOR_RED}Database error in get_sleep_data_for_bool_heatmap: {e}{COLOR_RESET}")
        return {}  # Return empty dict on database error
    finally:
        if conn:
            conn.close()
    return data