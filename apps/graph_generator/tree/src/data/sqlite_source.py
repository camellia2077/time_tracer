import sqlite3
from typing import Dict, List

from core.models import ProjectRecord

class TreeSQLiteSource:
    """Fetches project data from SQLite."""
    def __init__(self, db_path: str):
        self.db_path = db_path.strip()

    def fetch_projects(self) -> List[ProjectRecord]:
        if not self.db_path:
            print("Database path is empty. Configure it via config, env, or CLI.")
            return []

        conn = None
        try:
            conn = sqlite3.connect(self.db_path)
            cursor = conn.cursor()
            cursor.execute(
                "SELECT id, parent_id, name FROM projects ORDER BY name"
            )
            rows = cursor.fetchall()
            return [
                ProjectRecord(
                    id=row[0],
                    parent_id=row[1] if row[1] is not None else None,
                    name=row[2] if row[2] is not None else "",
                )
                for row in rows
            ]
        except sqlite3.Error as exc:
            print(f"Database error: {exc}")
            return []
        finally:
            if conn:
                conn.close()

    def fetch_project_durations(self) -> Dict[int, int]:
        if not self.db_path:
            print("Database path is empty. Configure it via config, env, or CLI.")
            return {}

        conn = None
        try:
            conn = sqlite3.connect(self.db_path)
            cursor = conn.cursor()
            cursor.execute(
                "SELECT project_id, COALESCE(SUM(duration), 0) FROM time_records GROUP BY project_id"
            )
            rows = cursor.fetchall()
            return {int(row[0]): int(row[1]) for row in rows if row[0] is not None}
        except sqlite3.Error as exc:
            print(f"Database error: {exc}")
            return {}
        finally:
            if conn:
                conn.close()
