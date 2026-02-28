# test/core/engine.py
import re
import shutil
import subprocess
import time
from pathlib import Path

from ..cases.version import VersionChecker
from ..conf.definitions import (
    AppExitCode,
    CommandSpec,
    GlobalConfig,
    TestContext,
    TestReport,
)
from ..infrastructure.environment import EnvironmentManager
from .base import BaseTester, TestCounter
from .reporter import Reporter

_ANSI_ESCAPE_RE = re.compile(r"\x1b\[[0-9;]*m")
_LEGACY_COMMAND_ALIASES = {
    "validate_structure": "validate-structure",
    "validate_logic": "validate-logic",
}


class TableTester(BaseTester):
    def __init__(
        self,
        counter: TestCounter,
        module_order: int,
        stage: str,
        context: TestContext,
        commands: list[CommandSpec],
        stop_on_failure: bool,
        show_output: str,
        log_routing_rules=None,
    ):
        super().__init__(
            counter,
            module_order,
            stage,
            context,
            show_output=show_output,
            log_routing_rules=log_routing_rules,
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
                raw_command=spec.raw_command,
                stdin_input=spec.stdin_input,
                expect_files=spec.expect_files,
                expect_file_contains=spec.expect_file_contains,
                expect_stdout_contains=spec.expect_stdout_contains,
                expect_stderr_contains=spec.expect_stderr_contains,
            )
            report.results.append(result)
            if self.stop_on_failure and result.status == "FAIL":
                break
        return report


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
        all_reports: list[TestReport] = []
        try:
            if (
                self.run_control.ENABLE_ENVIRONMENT_CLEAN
                or self.run_control.ENABLE_ENVIRONMENT_PREPARE
            ):
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

            if self.run_control.ENABLE_TEST_EXECUTION:
                if self.paths.PROCESSED_JSON_DIR:
                    self.paths.PROCESSED_JSON_DIR.mkdir(parents=True, exist_ok=True)
                context = self._build_context()
                counter = TestCounter()
                modules = self._build_table_suites(context, counter)
                all_reports = self._run_suite(modules)

            duration = time.monotonic() - self.start_time
            self.case_records = self.reporter.build_case_records(all_reports)
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

    @staticmethod
    def _strip_ansi_codes(text: str) -> str:
        if not text:
            return ""
        return _ANSI_ESCAPE_RE.sub("", text)

    @staticmethod
    def _normalize_command_name(name: str) -> str:
        return _LEGACY_COMMAND_ALIASES.get(name, name)

    @staticmethod
    def _is_cli_subcommand_token(token: str) -> bool:
        stripped = (token or "").strip()
        if not stripped:
            return False
        if stripped.startswith("-") or stripped.startswith("/"):
            return False
        return bool(re.match(r"^[a-z][a-z0-9-]*$", stripped))

    def _discover_available_commands(self, exe_path: Path) -> set[str]:
        if self._available_commands_cache is not None:
            return set(self._available_commands_cache)

        result = subprocess.run(
            [str(exe_path), "--help"],
            cwd=str(exe_path.parent),
            capture_output=True,
            text=True,
            check=False,
            encoding="utf-8",
            errors="ignore",
        )
        if result.returncode != 0:
            stderr_tail = self._strip_ansi_codes(result.stderr or "").strip()
            raise RuntimeError(
                f"Failed to fetch CLI command list via --help (exit={result.returncode}). "
                f"{stderr_tail}"
            )

        output_text = self._strip_ansi_codes((result.stdout or "") + "\n" + (result.stderr or ""))
        commands: set[str] = set()
        in_available_block = False
        for raw_line in output_text.splitlines():
            line = raw_line.strip()
            if line == "Available Commands:":
                in_available_block = True
                continue
            if not in_available_block:
                continue
            if line == "Global Options:":
                break
            if not line or line.endswith(":"):
                continue

            match = re.match(r"^([a-z][a-z0-9-]*)\s{2,}.+$", line)
            if match:
                commands.add(match.group(1))

        if not commands:
            raise RuntimeError("Failed to parse command list from CLI --help output.")

        self._available_commands_cache = commands
        return set(commands)

    def _validate_command_specs(self, context: TestContext, commands: list[CommandSpec]) -> None:
        errors: list[str] = []
        candidates: list[tuple[CommandSpec, str]] = []

        for spec in commands:
            if spec.raw_command:
                continue
            if not spec.args:
                errors.append(f"{spec.name}: empty args")
                continue

            command_name = spec.args[0].strip()
            if not self._is_cli_subcommand_token(command_name):
                continue

            candidates.append((spec, command_name))

        if candidates:
            available_commands = self._discover_available_commands(context.exe_path)
            for spec, command_name in candidates:
                if command_name not in available_commands:
                    if int(spec.expect_exit) == AppExitCode.COMMAND_NOT_FOUND:
                        continue
                    available_str = ", ".join(sorted(available_commands))
                    errors.append(
                        f"{spec.name}: unknown command '{command_name}' "
                        f"(available: {available_str})"
                    )

        if errors:
            details = "\n".join(f"  - {item}" for item in errors)
            raise ValueError(f"Command spec mismatch with CLI --help:\n{details}")

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
            expanded_commands = self._expand_commands(context, commands)
            self._validate_command_specs(context, expanded_commands)

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

    def _expand_commands(
        self, context: TestContext, commands: list[CommandSpec]
    ) -> list[CommandSpec]:
        variables = {
            "data_path": str(context.source_data_path),
            "db_path": str(context.db_path) if context.db_path else "",
            "output_dir": str(context.output_dir),
            "export_output_dir": str(context.export_output_dir)
            if context.export_output_dir
            else "",
            "exe_dir": str(context.exe_path.parent),
            "processed_json_path": str(context.processed_json_path)
            if context.processed_json_path
            else "",
            "processed_json_dir": str(context.processed_json_dir)
            if context.processed_json_dir
            else "",
        }

        def safe_format(value: str) -> str:
            class SafeDict(dict):
                def __missing__(self, key):
                    return "{" + key + "}"

            return value.format_map(SafeDict(variables))

        expanded = []
        for command in commands:
            formatted_args = [safe_format(str(arg)) for arg in command.args]
            if formatted_args and not command.raw_command:
                first_arg = formatted_args[0]
                if not first_arg.startswith("-"):
                    formatted_args[0] = self._normalize_command_name(first_arg)

            expanded.append(
                CommandSpec(
                    name=safe_format(command.name),
                    args=formatted_args,
                    stage=command.stage,
                    expect_exit=command.expect_exit,
                    raw_command=command.raw_command,
                    add_output_dir=command.add_output_dir,
                    stdin_input=safe_format(command.stdin_input) if command.stdin_input else None,
                    expect_files=[safe_format(str(path)) for path in command.expect_files],
                    expect_file_contains=[
                        safe_format(str(spec)) for spec in command.expect_file_contains
                    ],
                    expect_stdout_contains=[
                        safe_format(str(text)) for text in command.expect_stdout_contains
                    ],
                    expect_stderr_contains=[
                        safe_format(str(text)) for text in command.expect_stderr_contains
                    ],
                )
            )
        return expanded

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
