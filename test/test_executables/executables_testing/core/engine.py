# test/core/engine.py
import time
from typing import List
from pathlib import Path

from ..conf.definitions import GlobalConfig, TestContext, Colors, CommandSpec, TestReport
from ..infrastructure.environment import EnvironmentManager
from .base import BaseTester, TestCounter
from .reporter import Reporter

class TableTester(BaseTester):
    def __init__(self, counter: TestCounter, module_order: int, stage: str,
                 context: TestContext, commands: List[CommandSpec],
                 stop_on_failure: bool):
        super().__init__(counter, module_order, stage, context)
        self.commands = commands
        self.stop_on_failure = stop_on_failure

    def run_tests(self) -> TestReport:
        report = TestReport(module_name=self.module_name)
        for spec in self.commands:
            res = self.run_command_test(
                spec.name,
                spec.args,
                add_output_dir=spec.add_output_dir,
                expect_exit=spec.expect_exit,
                stdin_input=spec.stdin_input,
                expect_files=spec.expect_files,
                expect_stdout_contains=spec.expect_stdout_contains,
                expect_stderr_contains=spec.expect_stderr_contains
            )
            report.results.append(res)
            if self.stop_on_failure and res.status == "FAIL":
                break
        return report

class TestEngine:
    def __init__(self, config: GlobalConfig):
        self.cfg = config
        self.paths = config.paths
        self.cli_names = config.cli_names
        self.commands = config.commands
        self.run_control = config.run_control
        self.cleanup = config.cleanup
        
        self.start_time = 0.0
        self.env_manager = None
        self.reporter = Reporter() # [新增]

    def run(self):
        self.start_time = time.monotonic()
        
        # 1. 环境准备
        if self.run_control.ENABLE_ENVIRONMENT_CLEAN or self.run_control.ENABLE_ENVIRONMENT_PREPARE:
            use_temp = False 
            self.env_manager = EnvironmentManager(
                source_exe_dir=self.paths.SOURCE_EXECUTABLES_DIR,
                files_to_copy=self.cleanup.FILES_TO_COPY,
                folders_to_copy=self.cleanup.FOLDERS_TO_COPY,
                use_temp=use_temp
            )
            
            target_override = self.paths.TARGET_EXECUTABLES_DIR
            do_clean = self.run_control.ENABLE_ENVIRONMENT_CLEAN
            do_deploy = self.run_control.ENABLE_ENVIRONMENT_PREPARE
            
            final_exe_path = self.env_manager.setup(
                target_dir_override=target_override, 
                should_clean=do_clean,
                should_deploy=do_deploy
            )
            
            self.paths.TARGET_EXECUTABLES_DIR = final_exe_path
            
            # [删除] 移除了 self.paths.PY_OUTPUT_DIR.mkdir(...) 
            # 现在由 TestLogger 在 BaseTester 中实例化时创建

        # 2. 测试执行
        all_reports = []
        if self.run_control.ENABLE_TEST_EXECUTION:
            if self.paths.PROCESSED_JSON_DIR:
                self.paths.PROCESSED_JSON_DIR.mkdir(parents=True, exist_ok=True)
            ctx = self._build_context()
            counter = TestCounter()
            modules = self._build_table_suites(ctx, counter)
            all_reports = self._run_suite(modules)

        # 3. 总结
        duration = time.monotonic() - self.start_time
        # [修改] 调用 reporter 打印总结
        self.reporter.print_summary(all_reports, duration, self.paths.PY_OUTPUT_DIR)
        
        if self.env_manager:
            self.env_manager.teardown()
            
        # 只要有一个模块有失败，就返回 False
        return all(r.failed_count == 0 for r in all_reports)

    def _build_context(self) -> TestContext:
        """构建运行时的上下文对象"""
        output_dir = Path.cwd() / self.paths.OUTPUT_DIR_NAME if self.paths.OUTPUT_DIR_NAME else Path.cwd()
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
            processed_json_path=self.paths.PROCESSED_JSON_PATH
        )

    def _build_table_suites(self, ctx: TestContext, counter: TestCounter) -> List[BaseTester]:
        """将命令列表按 stage 分组为测试模块"""
        if not self.commands:
            return []

        stages: List[tuple[str, List[CommandSpec]]] = []
        stage_index = {}
        for cmd in self.commands:
            stage = cmd.stage or "commands"
            if stage not in stage_index:
                stage_index[stage] = len(stages)
                stages.append((stage, []))
            stages[stage_index[stage]][1].append(cmd)

        modules: List[BaseTester] = []
        for idx, (stage, commands) in enumerate(stages, 1):
            modules.append(TableTester(
                counter=counter,
                module_order=idx,
                stage=stage,
                context=ctx,
                commands=self._expand_commands(ctx, commands),
                stop_on_failure=self.run_control.STOP_ON_FAILURE
            ))
        return modules

    def _expand_commands(self, ctx: TestContext, commands: List[CommandSpec]) -> List[CommandSpec]:
        """替换命令参数中的占位符"""
        variables = {
            "data_path": str(ctx.source_data_path),
            "db_path": str(ctx.db_path) if ctx.db_path else "",
            "output_dir": str(ctx.output_dir),
            "export_output_dir": str(ctx.export_output_dir) if ctx.export_output_dir else "",
            "exe_dir": str(ctx.exe_path.parent),
            "processed_json_path": str(ctx.processed_json_path) if ctx.processed_json_path else "",
            "processed_json_dir": str(ctx.processed_json_dir) if ctx.processed_json_dir else "",
        }

        def safe_format(value: str) -> str:
            class SafeDict(dict):
                def __missing__(self, key):
                    return "{" + key + "}"
            return value.format_map(SafeDict(variables))

        expanded = []
        for cmd in commands:
            expanded.append(CommandSpec(
                name=safe_format(cmd.name),
                args=[safe_format(str(a)) for a in cmd.args],
                stage=cmd.stage,
                expect_exit=cmd.expect_exit,
                add_output_dir=cmd.add_output_dir,
                stdin_input=safe_format(cmd.stdin_input) if cmd.stdin_input else None,
                expect_files=[safe_format(str(p)) for p in cmd.expect_files],
                expect_stdout_contains=[safe_format(str(s)) for s in cmd.expect_stdout_contains],
                expect_stderr_contains=[safe_format(str(s)) for s in cmd.expect_stderr_contains],
            ))
        return expanded

    def _run_suite(self, modules: List[BaseTester]):
        """运行套件逻辑，返回报告列表"""
        print("\n" + "="*20 + " Starting Test Sequence " + "="*20)
        reports = []
        for i, module in enumerate(modules, 1):
            # [修改] Engine 调度打印模块开始
            self.reporter.print_module_start(i, module.module_name)
            
            # [修改] 获取报告
            report = module.run_tests()
            reports.append(report)
            
            # [修改] Engine 调度打印报告
            self.reporter.print_module_report(report)

            # [新增] 若当前模块有失败，立即终止后续模块
            if report.failed_count > 0:
                print(f"{Colors.RED}Stopping further tests due to failures in "
                      f"module '{report.module_name}'.{Colors.RESET}")
                break
            
        return reports
