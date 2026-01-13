import sqlite3
import datetime
from pathlib import Path
from typing import Dict, Any

class SQLiteSource:
    """负责所有与 SQLite 数据库的交互。"""
    def __init__(self, db_path: Path):
        self.db_path = db_path

    def _execute_query(self, query: str, params: tuple) -> list:
        """执行一个查询并返回所有结果。"""
        conn = None
        try:
            # sqlite3 支持接受 Path 对象
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
    
    # ... (其余方法 fetch_project_duration_data 和 fetch_boolean_data 逻辑保持不变)
    # 仅需保留原有的 SQL 逻辑即可，不需要修改 SQL 语句部分
    
    def fetch_project_duration_data(self, project_name: str, year: int) -> Dict[datetime.date, float]:
        # (保持你之前的修复版本，使用 RECURSIVE CTE 和 %Y-%m-%d)
        print(f"\n--- 正在查询项目 '{project_name}' 的时长数据 ---")
        query = """
        WITH RECURSIVE project_tree(id) AS (
            SELECT id FROM projects WHERE name = ?
            UNION ALL
            SELECT p.id FROM projects p
            JOIN project_tree pt ON p.parent_id = pt.id
        )
        SELECT tr.date, SUM(tr.duration) 
        FROM time_records tr
        JOIN project_tree pt ON tr.project_id = pt.id
        WHERE SUBSTR(tr.date, 1, 4) = ?
        GROUP BY tr.date;
        """
        params = (project_name, str(year))
        rows = self._execute_query(query, params)
        data = {}
        for row in rows:
            try:
                d = datetime.datetime.strptime(row[0], "%Y-%m-%d").date()
                data[d] = row[1] / 3600.0
            except (ValueError, TypeError):
                continue
        print(f"成功查询到 {len(data)} 天的数据。")
        return data

    def fetch_boolean_data(self, column_name: str, year: int) -> Dict[datetime.date, int]:
        # (保持你之前的修复版本)
        print(f"\n--- 正在查询布尔字段 '{column_name}' 的数据 ---")
        if column_name not in ["sleep", "status", "exercise"]:
            raise ValueError(f"不安全的列名: {column_name}")
        
        query = f"SELECT date, {column_name} FROM days WHERE SUBSTR(date, 1, 4) = ?;"
        rows = self._execute_query(query, (str(year),))
        data = {}
        for row in rows:
            try:
                d = datetime.datetime.strptime(row[0], "%Y-%m-%d").date()
                data[d] = row[1]
            except (ValueError, TypeError):
                continue
        print(f"成功查询到 {len(data)} 天的数据。")
        return data