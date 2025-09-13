import sqlite3
from typing import List, Tuple

class TimelineSQLiteSource:
    """负责所有与时间线数据相关的 SQLite 数据库交互。"""
    def __init__(self, db_path: str):
        self.db_path = db_path

    def fetch_records_for_date(self, target_date: str) -> List[Tuple]:
        """根据指定日期查询所有活动记录。"""
        print(f"\n--- 正在查询日期 '{target_date}' 的活动记录 ---")
        conn = None
        try:
            conn = sqlite3.connect(self.db_path)
            cursor = conn.cursor()

            sql_query = """
            SELECT start_timestamp, end_timestamp, project_path 
            FROM time_records 
            WHERE date = ? 
            ORDER BY logical_id ASC;
            """
            cursor.execute(sql_query, (target_date,))
            records = cursor.fetchall()

            if not records:
                print(f"在日期 {target_date} 没有找到任何活动记录。")
            else:
                print(f"成功查询到 {len(records)} 条记录。")
            
            return records

        except sqlite3.Error as e:
            print(f"数据库查询出错: {e}")
            return []
        finally:
            if conn:
                conn.close()