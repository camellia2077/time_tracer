import datetime
import json
import subprocess
from pathlib import Path
from typing import Dict, Optional

from .sqlite_source import SQLiteSource


class CoreContractSource:
    """Fetches heatmap data via core semantic-json contract."""

    def __init__(
        self,
        db_path: Path,
        cli_path: str = "time_tracer_cli",
        timeout_seconds: int = 20,
        fallback_sqlite: Optional[SQLiteSource] = None,
        allow_sql_fallback: bool = True,
    ):
        self.db_path = Path(db_path)
        self.cli_path = (cli_path or "time_tracer_cli").strip() or "time_tracer_cli"
        self.timeout_seconds = max(1, int(timeout_seconds))
        self.fallback_sqlite = fallback_sqlite
        self.allow_sql_fallback = allow_sql_fallback

    @staticmethod
    def _extract_json_payload(stdout: str) -> dict:
        text = (stdout or "").strip()
        if not text:
            raise RuntimeError("Core CLI returned empty output.")
        try:
            return json.loads(text)
        except json.JSONDecodeError:
            start = text.find("{")
            end = text.rfind("}")
            if start < 0 or end <= start:
                raise RuntimeError("Core CLI output is not valid JSON.")
            return json.loads(text[start : end + 1])

    def _build_report_chart_command(self, project_name: str, year: int) -> list[str]:
        from_date = f"{year:04d}0101"
        to_date = f"{year:04d}1231"
        return [
            self.cli_path,
            "query",
            "data",
            "report-chart",
            "--from",
            from_date,
            "--to",
            to_date,
            "--root",
            project_name,
            "--data-output",
            "json",
            "--database",
            str(self.db_path),
        ]

    def _query_report_chart_payload(self, project_name: str, year: int) -> dict:
        command = self._build_report_chart_command(project_name, year)
        result = subprocess.run(
            command,
            capture_output=True,
            text=True,
            check=False,
            timeout=self.timeout_seconds,
        )
        if result.returncode != 0:
            stderr = (result.stderr or "").strip()
            raise RuntimeError(
                f"Core CLI exited with code {result.returncode}. {stderr}"
            )

        payload = self._extract_json_payload(result.stdout)
        action = str(payload.get("action", "")).strip().replace("-", "_")
        if action != "report_chart":
            raise RuntimeError(f"Unexpected action in payload: {payload.get('action')}")
        return payload

    @staticmethod
    def _convert_series_to_daily_hours(payload: dict) -> Dict[datetime.date, float]:
        data: Dict[datetime.date, float] = {}
        series = payload.get("series", [])
        if not isinstance(series, list):
            return data

        for row in series:
            if not isinstance(row, dict):
                continue

            date_text = str(row.get("date", "")).strip()
            if not date_text:
                continue

            try:
                day = datetime.date.fromisoformat(date_text)
            except ValueError:
                continue

            try:
                seconds = float(row.get("duration_seconds", 0))
            except (TypeError, ValueError):
                seconds = 0.0

            if seconds < 0:
                seconds = 0.0
            data[day] = seconds / 3600.0
        return data

    def fetch_project_duration_data(
        self, project_name: str, year: int
    ) -> Dict[datetime.date, float]:
        print(f"\n--- 正在通过 Core 契约查询项目 '{project_name}' 的时长数据 ---")
        try:
            payload = self._query_report_chart_payload(project_name, year)
            data = self._convert_series_to_daily_hours(payload)
            print(f"通过 Core 契约查询到 {len(data)} 天的数据。")
            return data
        except Exception as exc:  # pylint: disable=broad-except
            print(f"Core 契约查询失败: {exc}")
            if self.allow_sql_fallback and self.fallback_sqlite is not None:
                print("回退到 SQLite 直连模式。")
                return self.fallback_sqlite.fetch_project_duration_data(
                    project_name, year
                )
            return {}

    def fetch_boolean_data(self, column_name: str, year: int) -> Dict[datetime.date, int]:
        # Core 当前没有 day-flag 热力图契约，先复用 SQL 路径，避免功能回退。
        if self.fallback_sqlite is None:
            print("布尔热力图暂未接入 Core 契约，且未配置 SQLite 回退。")
            return {}
        print("布尔热力图暂未接入 Core 契约，继续使用 SQLite 数据源。")
        return self.fallback_sqlite.fetch_boolean_data(column_name, year)

