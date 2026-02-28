import datetime
import subprocess
import sys
import unittest
from pathlib import Path
from unittest import mock


HEATMAP_ROOT = Path(__file__).resolve().parents[1]
if str(HEATMAP_ROOT) not in sys.path:
    sys.path.insert(0, str(HEATMAP_ROOT))

from heatmap_app.data.core_contract_source import CoreContractSource  # noqa: E402


class _StubSQLiteSource:
    def __init__(self):
        self.project_calls = 0
        self.boolean_calls = 0

    def fetch_project_duration_data(self, project_name: str, year: int):
        self.project_calls += 1
        return {datetime.date(year, 1, 1): 2.5}

    def fetch_boolean_data(self, column_name: str, year: int):
        self.boolean_calls += 1
        return {datetime.date(year, 1, 2): 1}


class CoreContractSourceTests(unittest.TestCase):
    @mock.patch("heatmap_app.data.core_contract_source.subprocess.run")
    def test_fetch_project_duration_data_from_contract(self, mock_run):
        mock_run.return_value = subprocess.CompletedProcess(
            args=["time_tracer_cli"],
            returncode=0,
            stdout=(
                '{"schema_version":1,"action":"report_chart","output_mode":"semantic_json",'
                '"series":[{"date":"2026-01-01","duration_seconds":3600}]}'
            ),
            stderr="",
        )

        source = CoreContractSource(Path("demo.sqlite3"), cli_path="time_tracer_cli")
        data = source.fetch_project_duration_data("study", 2026)

        self.assertIn(datetime.date(2026, 1, 1), data)
        self.assertAlmostEqual(data[datetime.date(2026, 1, 1)], 1.0)
        called_args = mock_run.call_args[0][0]
        self.assertIn("--data-output", called_args)
        self.assertIn("json", called_args)
        self.assertIn("--root", called_args)
        self.assertIn("study", called_args)

    @mock.patch("heatmap_app.data.core_contract_source.subprocess.run")
    def test_fetch_project_duration_data_falls_back_to_sqlite(self, mock_run):
        mock_run.return_value = subprocess.CompletedProcess(
            args=["time_tracer_cli"],
            returncode=2,
            stdout="",
            stderr="failed",
        )
        fallback = _StubSQLiteSource()
        source = CoreContractSource(
            Path("demo.sqlite3"),
            cli_path="time_tracer_cli",
            fallback_sqlite=fallback,
            allow_sql_fallback=True,
        )

        data = source.fetch_project_duration_data("study", 2026)
        self.assertEqual(fallback.project_calls, 1)
        self.assertIn(datetime.date(2026, 1, 1), data)

    def test_boolean_data_uses_sqlite_fallback(self):
        fallback = _StubSQLiteSource()
        source = CoreContractSource(
            Path("demo.sqlite3"),
            fallback_sqlite=fallback,
        )

        data = source.fetch_boolean_data("sleep", 2026)
        self.assertEqual(fallback.boolean_calls, 1)
        self.assertEqual(data.get(datetime.date(2026, 1, 2)), 1)


if __name__ == "__main__":
    unittest.main()

