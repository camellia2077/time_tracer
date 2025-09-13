# graph_generator/data/db_access.py
import sqlite3
import pandas as pd
import datetime
import os
import sys
from typing import Dict, Tuple, Optional

# --- [核心修改] 从config模块导入颜色常量 ---
from core.config import COLOR_GREEN, COLOR_RED, COLOR_RESET

# --- Configuration ---
DB_PATH = 'time_data.db'
DB_CONNECTION_TIMEOUT = 10
MAX_RECURSION_DEPTH = 4 

# --- Connection Management ---
def _get_db_connection() -> Optional[sqlite3.Connection]:
    if not os.path.exists(DB_PATH):
        print(f"{COLOR_RED}Error: Database file not found at '{DB_PATH}'{COLOR_RESET}")
        return None
    try:
        return sqlite3.connect(DB_PATH, timeout=DB_CONNECTION_TIMEOUT)
    except sqlite3.Error as e:
        print(f"{COLOR_RED}Database connection error: {e}{COLOR_RESET}")
        return None

# --- Public Data Access Functions ---
def get_data_for_timeline() -> Optional[Tuple[pd.DataFrame, pd.DataFrame, pd.DataFrame]]:
    conn = _get_db_connection()
    if not conn:
        return None
    try:
        df_days = pd.read_sql_query("SELECT * FROM days", conn)
        df_records = pd.read_sql_query("SELECT * FROM time_records", conn)
        df_parents = pd.read_sql_query("SELECT * FROM parent_child", conn)
        return df_days, df_records, df_parents
    except pd.io.sql.DatabaseError as e:
        print(f"{COLOR_RED}Error reading from database: {e}{COLOR_RESET}")
        return None
    finally:
        if conn:
            conn.close()

def get_data_for_heatmap(year: int, project_name: str) -> Dict[datetime.date, float]:
    project_data = {}
    conn = _get_db_connection()
    if not conn:
        return {}
        
    try:
        sql_query = f"""
        WITH RECURSIVE target_projects(project) AS (
          VALUES(?)
          UNION ALL
          SELECT pc.child FROM parent_child pc JOIN target_projects tp ON pc.parent = tp.project
        )
        SELECT tr.date, SUM(tr.duration)
        FROM time_records tr
        WHERE tr.project_path IN (SELECT project FROM target_projects)
          AND SUBSTR(tr.date, 1, 4) = ?
        GROUP BY tr.date;
        """
        params = (project_name, str(year))
        df = pd.read_sql_query(sql_query, conn, params=params)

        for _, row in df.iterrows():
            total_seconds = row['SUM(tr.duration)']
            if total_seconds is not None and total_seconds > 0:
                current_date = datetime.datetime.strptime(row['date'], '%Y%m%d').date()
                project_data[current_date] = total_seconds / 3600.0
                
    except (sqlite3.Error, pd.io.sql.DatabaseError) as e:
        print(f"{COLOR_RED}An error occurred during database operation: {e}{COLOR_RESET}", file=sys.stderr)
        return {}
    finally:
        if conn:
            conn.close()
            
    return project_data

def get_sleep_data_for_bool_heatmap(year: int) -> Optional[Dict[datetime.date, str]]:
    data: Dict[datetime.date, str] = {}
    conn = _get_db_connection()
    if not conn:
        return None
    try:
        query = "SELECT date, sleep FROM days WHERE SUBSTR(date, 1, 4) = ?"
        df = pd.read_sql_query(query, conn, params=(str(year),))
        for _, row in df.iterrows():
            try:
                day_date = datetime.datetime.strptime(row['date'], '%Y%m%d').date()
                # 将数据库中的 0/1 转换为 'False'/'True' 字符串
                data[day_date] = str(bool(row['sleep']))
            except (ValueError, TypeError):
                print(f"Warning: Skipping invalid data for date '{row['date']}' in database.")
    except (sqlite3.Error, pd.io.sql.DatabaseError) as e:
        print(f"{COLOR_RED}Database error in get_sleep_data_for_bool_heatmap: {e}{COLOR_RESET}")
        return {}
    finally:
        if conn:
            conn.close()
    return data