# test/core/engine.py
import time
from typing import List
from pathlib import Path

from ..conf.definitions import GlobalConfig, TestContext, Colors
from ..infrastructure.environment import EnvironmentManager
from .base import BaseTester, TestCounter  # 同级 core 目录下
from .registry import create_test_suite    # 同级 core 目录下

class TestEngine:
    def __init__(self, config: GlobalConfig):
        self.cfg = config
        self.paths = config.paths
        self.cli_names = config.cli_names
        self.run_control = config.run_control
        self.cleanup = config.cleanup
        
        self.start_time = 0.0
        self.env_manager = None

    def run(self):
        self.start_time = time.monotonic()
        
        # 默认目标路径
        final_exe_path = self.paths.TARGET_EXECUTABLES_DIR

        # 1. 环境准备
        if self.run_control.ENABLE_ENVIRONMENT_CLEAN or self.run_control.ENABLE_ENVIRONMENT_PREPARE:
            # [修改] 强制不使用临时目录，始终使用 config.toml 中定义的 target_executables_dir
            # 即便 enable_environment_clean=1，也只代表我们需要“准备/覆盖”环境，而不是“创建临时”环境
            use_temp = False 

            self.env_manager = EnvironmentManager(
                source_exe_dir=self.paths.SOURCE_EXECUTABLES_DIR,
                files_to_copy=self.cleanup.FILES_TO_COPY,
                folders_to_copy=self.cleanup.FOLDERS_TO_COPY,
                use_temp=use_temp
            )
            
            # 明确指定目标路径为配置文件中的路径
            target_override = self.paths.TARGET_EXECUTABLES_DIR
            final_exe_path = self.env_manager.setup(target_dir_override=target_override)
            
            # 更新路径配置
            self.paths.TARGET_EXECUTABLES_DIR = final_exe_path
            
            # [修改] 如果不使用临时目录，日志路径将直接使用 config.toml 中定义的 PY_OUTPUT_DIR，
            # 不需要像之前那样在这里动态重写 self.paths.PY_OUTPUT_DIR。
            # 确保日志目录存在
            if self.paths.PY_OUTPUT_DIR:
                self.paths.PY_OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

        # 2. 测试执行
        all_passed = True
        if self.run_control.ENABLE_TEST_EXECUTION:
            ctx = self._build_context()
            counter = TestCounter()
            modules = create_test_suite(self.cfg, ctx, counter)
            all_passed = self._run_suite(modules)

        # 3. 总结
        duration = time.monotonic() - self.start_time
        self._print_summary(all_passed, duration)
        
        # 可选：清理
        if self.env_manager:
            self.env_manager.teardown()
            
        return all_passed

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

    def _run_suite(self, modules: List[BaseTester]) -> bool:
        """运行套件逻辑"""
        print("\n" + "="*20 + " Starting Test Sequence " + "="*20)
        suite_passed = True
        for i, module in enumerate(modules, 1):
            print(f"{Colors.CYAN}--- {i}. Running {module.module_name} Tasks ---{Colors.RESET}")
            if not module.run_tests():
                suite_passed = False
                print(f"\n{Colors.RED}注意: 模块 '{module.module_name}' 未完全通过。{Colors.RESET}")
        return suite_passed

    def _print_summary(self, passed: bool, duration: float):
        if passed:
            print(f"\n{Colors.GREEN}✅ All test steps completed successfully!{Colors.RESET}")
        else:
            print(f"\n{Colors.RED}❌ Some test steps failed. Check logs in: {self.paths.PY_OUTPUT_DIR}{Colors.RESET}")
        print(f"{Colors.CYAN}Total execution time: {duration:.2f} seconds.{Colors.RESET}")