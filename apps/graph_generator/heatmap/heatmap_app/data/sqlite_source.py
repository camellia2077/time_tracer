import sqlite3
import datetime
from typing import Dict

class SQLiteSource:
    """负责所有与 SQLite 数据库的交互。"""
    def __init__(self, db_path: str):
        self.db_path = db_path

    def _execute_query(self, query: str, params: tuple) -> list:
        """执行一个查询并返回所有结果。"""
        conn = None
        try:
            conn = sqlite3.connect(self.db_path)
            cursor = conn.cursor()
            cursor.execute(query, params)
            return cursor.fetchall()
        except sqlite3.Error as e:
            print(f"数据库查询出错: {e}")
            return []
        finally:
            if conn:
                conn.close()

    def fetch_project_duration_data(self, project_name: str, year: int) -> Dict[datetime.date, float]:
        """从数据库获取数值型项目数据（如时长）。"""
        print(f"\n--- 正在查询项目 '{project_name}' 的时长数据 ---")
        query = """
        SELECT date, SUM(duration) FROM time_records
        WHERE project_path LIKE ? AND SUBSTR(date, 1, 4) = ?
        GROUP BY date;
        """
        params = (f"{project_name}%", str(year))
        rows = self._execute_query(query, params)
        
        data = {
            datetime.datetime.strptime(row[0], "%Y%m%d").date(): row[1] / 3600.0
            for row in rows
        }
        print(f"成功查询到 {len(data)} 天的数据。")
        return data

    def fetch_boolean_data(self, column_name: str, year: int) -> Dict[datetime.date, int]:
        """从数据库获取布尔型数据 (0或1)。"""
        print(f"\n--- 正在查询布尔字段 '{column_name}' 的数据 ---")
        if column_name not in ["sleep", "status", "exercise"]:
            raise ValueError(f"不安全的列名: {column_name}")
        
        query = f"SELECT date, {column_name} FROM days WHERE SUBSTR(date, 1, 4) = ?;"
        rows = self._execute_query(query, (str(year),))

        data = {
            datetime.datetime.strptime(row[0], "%Y%m%d").date(): row[1]
            for row in rows
        }
        print(f"成功查询到 {len(data)} 天的数据。")
        return data