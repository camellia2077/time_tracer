# test/core/engine.py
import time
from typing import List
from pathlib import Path

from ..conf.definitions import GlobalConfig, TestContext, Colors
from ..infrastructure.environment import EnvironmentManager
from .base import BaseTester, TestCounter
from .registry import create_test_suite
from .reporter import Reporter  # [新增]

class TestEngine:
    def __init__(self, config: GlobalConfig):
        self.cfg = config
        self.paths = config.paths
        self.cli_names = config.cli_names
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
            ctx = self._build_context()
            counter = TestCounter()
            modules = create_test_suite(self.cfg, ctx, counter)
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
            py_output_base_dir=self.paths.PY_OUTPUT_DIR,
            processed_dir_name=self.paths.PROCESSED_DATA_DIR_NAME,
            processed_json_path=self.paths.PROCESSED_JSON_PATH
        )

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
            
        return reports