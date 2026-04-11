import sys
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import TestCase

REPO_ROOT = Path(__file__).resolve().parents[4]
TEST_FRAMEWORK_DIR = REPO_ROOT / "test" / "framework"

if str(TEST_FRAMEWORK_DIR) not in sys.path:
    sys.path.insert(0, str(TEST_FRAMEWORK_DIR))

from core.conf.definitions import (  # noqa: E402
    CLINames,
    Cleanup,
    GlobalConfig,
    LogRoutingConfig,
    Paths,
    PipelineConfig,
    RunControl,
    TestParams,
)
from core.suite.engine import TestEngine  # noqa: E402


class TestTestEngineLogCleanup(TestCase):
    def test_prepare_environment_cleans_existing_python_log_directories(self) -> None:
        with TemporaryDirectory() as tmp:
            root = Path(tmp)
            exe_dir = root / "workspace"
            logs_dir = root / "logs"
            artifacts_dir = root / "artifacts"

            old_stage_dir = logs_dir / "4_version_check"
            old_stage_dir.mkdir(parents=True, exist_ok=True)
            (old_stage_dir / "legacy.log").write_text("old", encoding="utf-8")
            (logs_dir / "python_output.latest.log").write_text("active", encoding="utf-8")
            (logs_dir / "output.log").write_text("stale summary", encoding="utf-8")
            (artifacts_dir / "stale.txt").parent.mkdir(parents=True, exist_ok=True)
            (artifacts_dir / "stale.txt").write_text("stale", encoding="utf-8")

            config = GlobalConfig(
                paths=Paths(
                    SOURCE_EXECUTABLES_DIR=root / "source_bin",
                    TARGET_EXECUTABLES_DIR=exe_dir,
                    PY_OUTPUT_DIR=logs_dir,
                    OUTPUT_DIR_NAME=artifacts_dir,
                    SOURCE_DATA_PATH=root / "data",
                    DB_DIR=root / "db.sqlite3",
                    EXPORT_OUTPUT_DIR=root / "export",
                    PROCESSED_DATA_DIR_NAME="processed",
                ),
                cli_names=CLINames(EXECUTABLE_CLI_NAME="time_tracer_cli.exe"),
                test_params=TestParams(),
                cleanup=Cleanup(),
                log_routing=LogRoutingConfig(),
                run_control=RunControl(
                    ENABLE_ENVIRONMENT_CLEAN=True,
                    ENABLE_ENVIRONMENT_PREPARE=False,
                    ENABLE_TEST_EXECUTION=False,
                ),
                pipeline=PipelineConfig(),
            )

            engine = TestEngine(config, options={"no_color": True, "concise": True})
            engine._prepare_environment()

            self.assertTrue(logs_dir.exists())
            self.assertEqual(
                sorted(item.name for item in logs_dir.iterdir()),
                ["python_output.latest.log"],
            )
            self.assertTrue(artifacts_dir.exists())
            self.assertEqual(list(artifacts_dir.iterdir()), [])
