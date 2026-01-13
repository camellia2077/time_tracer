import sqlite3
import datetime
from typing import List, Tuple

class TimelineSQLiteSource:
    """负责所有与时间线数据相关的 SQLite 数据库交互。"""
    def __init__(self, db_path: str):
        self.db_path = db_path

    def fetch_records_for_date(self, target_date: str) -> List[Tuple]:
        """根据指定日期查询所有活动记录。"""
        # [兼容性修复] 检查日期格式，如果是 YYYYMMDD 则转换为 YYYY-MM-DD
        if len(target_date) == 8 and target_date.isdigit():
            target_date = f"{target_date[:4]}-{target_date[4:6]}-{target_date[6:]}"
            
        print(f"\n--- 正在查询日期 '{target_date}' 的活动记录 ---")
        conn = None
        try:
            conn = sqlite3.connect(self.db_path)
            cursor = conn.cursor()

            # [核心修复] 使用递归查询重组 project_path
            # 1. path_cte: 从当前项目向上递归查找父项目，拼接名称
            # 2. time_records: 关联计算出的完整路径
            sql_query = """
            WITH RECURSIVE path_cte(original_id, current_parent_id, full_path) AS (
                -- 基础情况：选择所有项目作为潜在的叶子节点/起始点
                SELECT id, parent_id, name 
                FROM projects
                
                UNION ALL
                
                -- 递归步骤：如果当前节点有父节点，将父节点名称拼接到前面
                SELECT cte.original_id, p.parent_id, p.name || '_' || cte.full_path
                FROM path_cte cte
                JOIN projects p ON cte.current_parent_id = p.id
            )
            SELECT 
                tr.start_timestamp, 
                tr.end_timestamp, 
                cte.full_path 
            FROM time_records tr
            JOIN path_cte cte ON tr.project_id = cte.original_id
            WHERE tr.date = ? 
              AND cte.current_parent_id IS NULL -- 只选择已经回溯到根节点的完整路径
            ORDER BY tr.logical_id ASC;
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