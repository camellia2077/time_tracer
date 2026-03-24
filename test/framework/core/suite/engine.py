# test/core/engine.py
import shutil
import time
from pathlib import Path

from ..cases.version import VersionChecker
from ..conf.definitions import (
    CommandSpec,
    GlobalConfig,
    TestContext,
    TestReport,
)
from ..infrastructure.environment import EnvironmentManager
from ..base import BaseTester, TestCounter
from ..command_expander import expand_commands
from ..command_validation import validate_command_specs
from .reporter import Reporter
from .table_tester import TableTester


class TestEngine:
    def __init__(self, config: GlobalConfig, options: dict | None = None):
        self.cfg = config
        self.paths = config.paths
        self.cli_names = config.cli_names
        self.commands = config.commands
        self.run_control = config.run_control
        self.cleanup = config.cleanup
        self.log_routing = config.log_routing
        self.options = options or {}

        self.start_time = 0.0
        self.env_manager = None
        self.show_output = self.options.get("show_output", "none")
        self.reporter = Reporter(
            no_color=bool(self.options.get("no_color", False)),
            concise=bool(self.options.get("concise", True)),
        )
        self.run_result = {
            "success": False,
            "exit_code": 1,
            "total_tests": 0,
            "total_failed": 0,
            "duration_seconds": 0.0,
            "log_dir": str(self.paths.PY_OUTPUT_DIR),
            "modules": [],
            "failed_cases": [],
            "error_message": "",
        }
        self.case_records: list[dict] = []
        self._available_commands_cache: set[str] | None = None

    def run(self):
        self.start_time = time.monotonic()
        try:
            self._prepare_environment()
            all_reports = self._run_enabled_test_modules()
            self._finalize_run_result(all_reports)
            return bool(self.run_result["success"])
        finally:
            if self.env_manager:
                self.env_manager.teardown()

    def _prepare_environment(self) -> None:
        if not (
            self.run_control.ENABLE_ENVIRONMENT_CLEAN or self.run_control.ENABLE_ENVIRONMENT_PREPARE
        ):
            return

        self.env_manager = EnvironmentManager(
            source_exe_dir=self.paths.SOURCE_EXECUTABLES_DIR,
            files_to_copy=self.cleanup.FILES_TO_COPY,
            folders_to_copy=self.cleanup.FOLDERS_TO_COPY,
            use_temp=False,
        )
        final_exe_path = self.env_manager.setup(
            target_dir_override=self.paths.TARGET_EXECUTABLES_DIR,
            should_clean=self.run_control.ENABLE_ENVIRONMENT_CLEAN,
            should_deploy=self.run_control.ENABLE_ENVIRONMENT_PREPARE,
        )
        self.paths.TARGET_EXECUTABLES_DIR = final_exe_path
        if self.run_control.ENABLE_ENVIRONMENT_CLEAN:
            self._clean_artifacts_output()
            self._clean_additional_directories()

    def _run_enabled_test_modules(self) -> list[TestReport]:
        if not self.run_control.ENABLE_TEST_EXECUTION:
            return []
        if self.paths.PROCESSED_JSON_DIR:
            self.paths.PROCESSED_JSON_DIR.mkdir(parents=True, exist_ok=True)
        context = self._build_context()
        counter = TestCounter()
        modules = self._build_table_suites(context, counter)
        return self._run_suite(modules)

    def _finalize_run_result(self, all_reports: list[TestReport]) -> None:
        duration = time.monotonic() - self.start_time
        self.case_records = self.reporter.build_case_records(all_reports)
        self.run_result = self.reporter.build_summary(
            all_reports,
            duration,
            self.paths.PY_OUTPUT_DIR,
        )
        self.reporter.print_summary(self.run_result)

    def _clean_additional_directories(self) -> None:
        for raw_path in self.cleanup.DIRECTORIES_TO_CLEAN:
            normalized = str(raw_path).strip()
            if not normalized:
                continue
            path = Path(normalized)
            self._clean_directory_contents(path)

    def _clean_artifacts_output(self) -> None:
        if not self.paths.OUTPUT_DIR_NAME:
            return
        self._clean_directory_contents(self.paths.OUTPUT_DIR_NAME)

    @staticmethod
    def _clean_directory_contents(path: Path) -> None:
        path.mkdir(parents=True, exist_ok=True)
        print(f"  Cleaning configured directory: {path}")
        for item in path.iterdir():
            try:
                if item.is_dir():
                    shutil.rmtree(item)
                else:
                    item.unlink()
            except Exception as error:
                raise RuntimeError(f"Failed to clean '{item}': {error}") from error

    def get_result(self) -> dict:
        return dict(self.run_result)

    def get_case_records(self) -> list[dict]:
        return list(self.case_records)

    def _build_context(self) -> TestContext:
        output_dir = self.paths.OUTPUT_DIR_NAME if self.paths.OUTPUT_DIR_NAME else Path.cwd()
        exe_path = self.paths.TARGET_EXECUTABLES_DIR / self.cli_names.EXECUTABLE_CLI_NAME

        return TestContext(
            exe_path=exe_path,
            source_data_path=self.paths.SOURCE_DATA_PATH,
            output_dir=output_dir,
            db_path=self.paths.DB_DIR,
            export_output_dir=self.paths.EXPORT_OUTPUT_DIR,
            processed_json_dir=self.paths.PROCESSED_JSON_DIR,
            py_output_base_dir=self.paths.PY_OUTPUT_DIR,
            processed_dir_name=self.paths.PROCESSED_DATA_DIR_NAME,
            processed_json_path=self.paths.PROCESSED_JSON_PATH,
        )

    def _build_table_suites(self, context: TestContext, counter: TestCounter) -> list[BaseTester]:
        if not self.commands:
            return []

        stages: list[tuple[str, list[CommandSpec]]] = []
        stage_index = {}
        for command in self.commands:
            stage = command.stage or "commands"
            if stage not in stage_index:
                stage_index[stage] = len(stages)
                stages.append((stage, []))
            stages[stage_index[stage]][1].append(command)

        modules: list[BaseTester] = []
        for idx, (stage, commands) in enumerate(stages, 1):
            expanded_commands = expand_commands(context, commands)
            self._available_commands_cache = validate_command_specs(
                context,
                expanded_commands,
                self._available_commands_cache,
            )

            if stage == "version":
                modules.append(
                    VersionChecker(
                        counter=counter,
                        module_order=idx,
                        context=context,
                        commands=expanded_commands,
                        show_output=self.show_output,
                        log_routing_rules=self.log_routing.rules,
                    )
                )
                continue

            modules.append(
                TableTester(
                    counter=counter,
                    module_order=idx,
                    stage=stage,
                    context=context,
                    commands=expanded_commands,
                    stop_on_failure=self.run_control.STOP_ON_FAILURE,
                    show_output=self.show_output,
                    log_routing_rules=self.log_routing.rules,
                )
            )
        return modules

    def _run_suite(self, modules: list[BaseTester]):
        print("\n" + "=" * 20 + " Starting Test Sequence " + "=" * 20)
        reports = []
        for index, module in enumerate(modules, 1):
            self.reporter.print_module_start(index, module.module_name)
            module_start = time.monotonic()
            report = module.run_tests()
            report.duration = time.monotonic() - module_start
            reports.append(report)
            self.reporter.print_module_report(report)

            if report.failed_count > 0:
                print(
                    f"Stopping further tests due to failures in module '{report.module_name}'.",
                    flush=True,
                )
                break
        return reports
