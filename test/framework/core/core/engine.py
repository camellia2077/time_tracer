# test/core/engine.py
import time
from pathlib import Path
from typing import Dict, List, Optional

from ..cases.version import VersionChecker
from ..conf.definitions import (CommandSpec, GlobalConfig, TestContext,
                                 TestReport)
from ..infrastructure.environment import EnvironmentManager
from .base import BaseTester, TestCounter
from .reporter import Reporter


class TableTester(BaseTester):
    def __init__(self, counter: TestCounter, module_order: int, stage: str,
                 context: TestContext, commands: List[CommandSpec],
                 stop_on_failure: bool, show_output: str):
        super().__init__(
            counter,
            module_order,
            stage,
            context,
            show_output=show_output,
        )
        self.commands = commands
        self.stop_on_failure = stop_on_failure

    def run_tests(self) -> TestReport:
        report = TestReport(module_name=self.module_name)
        for spec in self.commands:
            result = self.run_command_test(
                spec.name,
                spec.args,
                add_output_dir=spec.add_output_dir,
                expect_exit=spec.expect_exit,
                stdin_input=spec.stdin_input,
                expect_files=spec.expect_files,
                expect_stdout_contains=spec.expect_stdout_contains,
                expect_stderr_contains=spec.expect_stderr_contains,
            )
            report.results.append(result)
            if self.stop_on_failure and result.status == "FAIL":
                break
        return report


class TestEngine:
    def __init__(self, config: GlobalConfig, options: Optional[Dict] = None):
        self.cfg = config
        self.paths = config.paths
        self.cli_names = config.cli_names
        self.commands = config.commands
        self.run_control = config.run_control
        self.cleanup = config.cleanup
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

    def run(self):
        self.start_time = time.monotonic()
        all_reports: List[TestReport] = []
        try:
            if self.run_control.ENABLE_ENVIRONMENT_CLEAN or \
                    self.run_control.ENABLE_ENVIRONMENT_PREPARE:
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

            if self.run_control.ENABLE_TEST_EXECUTION:
                if self.paths.PROCESSED_JSON_DIR:
                    self.paths.PROCESSED_JSON_DIR.mkdir(parents=True,
                                                        exist_ok=True)
                context = self._build_context()
                counter = TestCounter()
                modules = self._build_table_suites(context, counter)
                all_reports = self._run_suite(modules)

            duration = time.monotonic() - self.start_time
            self.run_result = self.reporter.build_summary(
                all_reports,
                duration,
                self.paths.PY_OUTPUT_DIR,
            )
            self.reporter.print_summary(self.run_result)
            return bool(self.run_result["success"])
        finally:
            if self.env_manager:
                self.env_manager.teardown()

    def get_result(self) -> Dict:
        return dict(self.run_result)

    def _build_context(self) -> TestContext:
        output_dir = self.paths.OUTPUT_DIR_NAME \
            if self.paths.OUTPUT_DIR_NAME else Path.cwd()
        exe_path = self.paths.TARGET_EXECUTABLES_DIR / \
            self.cli_names.EXECUTABLE_CLI_NAME

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

    def _build_table_suites(self, context: TestContext,
                            counter: TestCounter) -> List[BaseTester]:
        if not self.commands:
            return []

        stages: List[tuple[str, List[CommandSpec]]] = []
        stage_index = {}
        for command in self.commands:
            stage = command.stage or "commands"
            if stage not in stage_index:
                stage_index[stage] = len(stages)
                stages.append((stage, []))
            stages[stage_index[stage]][1].append(command)

        modules: List[BaseTester] = []
        for idx, (stage, commands) in enumerate(stages, 1):
            if stage == "version":
                modules.append(VersionChecker(
                    counter=counter,
                    module_order=idx,
                    context=context,
                    show_output=self.show_output,
                ))
                continue

            modules.append(TableTester(
                counter=counter,
                module_order=idx,
                stage=stage,
                context=context,
                commands=self._expand_commands(context, commands),
                stop_on_failure=self.run_control.STOP_ON_FAILURE,
                show_output=self.show_output,
            ))
        return modules

    def _expand_commands(self, context: TestContext,
                         commands: List[CommandSpec]) -> List[CommandSpec]:
        variables = {
            "data_path": str(context.source_data_path),
            "db_path": str(context.db_path) if context.db_path else "",
            "output_dir": str(context.output_dir),
            "export_output_dir": str(context.export_output_dir)
            if context.export_output_dir else "",
            "exe_dir": str(context.exe_path.parent),
            "processed_json_path": str(context.processed_json_path)
            if context.processed_json_path else "",
            "processed_json_dir": str(context.processed_json_dir)
            if context.processed_json_dir else "",
        }

        def safe_format(value: str) -> str:
            class SafeDict(dict):
                def __missing__(self, key):
                    return "{" + key + "}"

            return value.format_map(SafeDict(variables))

        expanded = []
        for command in commands:
            expanded.append(CommandSpec(
                name=safe_format(command.name),
                args=[safe_format(str(arg)) for arg in command.args],
                stage=command.stage,
                expect_exit=command.expect_exit,
                add_output_dir=command.add_output_dir,
                stdin_input=safe_format(command.stdin_input)
                if command.stdin_input else None,
                expect_files=[safe_format(str(path))
                              for path in command.expect_files],
                expect_stdout_contains=[safe_format(str(text))
                                        for text in
                                        command.expect_stdout_contains],
                expect_stderr_contains=[safe_format(str(text))
                                        for text in
                                        command.expect_stderr_contains],
            ))
        return expanded

    def _run_suite(self, modules: List[BaseTester]):
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
                    f"Stopping further tests due to failures in "
                    f"module '{report.module_name}'.",
                    flush=True,
                )
                break
        return reports
