# builder/core.py

import sys
import time
import os
from pathlib import Path

# 更新导入路径以匹配新的分包结构
from .conf.settings import AppConfig
from .steps import tasks, commands
from .ui.console import print_header, print_error, print_success
from .ui.colors import AnsiColors

class BuildPipeline:
    """构建流水线核心控制器"""
    
    def __init__(self, project_dir: Path, script_dir: Path):
        self.project_dir = project_dir
        self.script_dir = script_dir
        self.config = None
        self.options = {"package": False, "install": False, "clean": False, "compiler": "default"}

    def run(self, argv: list):
        """执行构建流水线"""
        start_time = time.monotonic()
        build_dir_name = os.environ.get("TIME_TRACER_BUILD_DIR", "build")
        
        print_header(f"Working in project directory: {self.project_dir}")

        try:
            # 1. 加载配置
            self._load_config()

            # 2. 解析参数
            self.options, cmake_args = tasks.parse_arguments(
                argv, 
                default_compiler=self.config.COMPILER
            )

            # 3. 准备环境
            build_dir = tasks.prepare_build_directory(
                self.project_dir, 
                build_dir_name, 
                self.options["clean"]
            )
            
            # 切换工作目录
            os.chdir(build_dir)

            # 4. 执行核心构建步骤
            self._execute_steps(cmake_args)

        except Exception as e:
            print_error(f"\n!!! Build Pipeline Failed: {e}")
            sys.exit(1)
        finally:
            self._print_summary(start_time, build_dir_name)

    def _load_config(self):
        """加载 config.toml"""
        config_path = self.script_dir / "config.toml"
        try:
            self.config = AppConfig(config_path)
        except Exception as e:
            raise RuntimeError(f"Failed to load configuration: {e}")

    def _execute_steps(self, cmake_args):
        """按顺序执行构建命令"""
        # CMake 配置
        commands.run_cmake(
            self.options["package"], 
            cmake_args, 
            self.options["compiler"], 
            config=self.config,
            no_opt=self.options.get("no_opt", False),
            no_tidy=self.options.get("no_tidy", False)  # [新增] 传递 no_tidy 参数
        )
        
        # 编译
        commands.run_build()

        # 打包 (可选)
        installer_file = None
        if self.options["package"]:
            installer_file = commands.run_cpack()
        
        # 安装/运行 (可选)
        if self.options["install"]:
            commands.run_installer(installer_file)

    def _print_summary(self, start_time, build_dir_name):
        """打印最终统计信息"""
        end_time = time.monotonic()
        duration = int(end_time - start_time)
        minutes, seconds = divmod(duration, 60)

        print("\n" + "="*60)
        print_success(f"{AnsiColors.BOLD}Process finished!{AnsiColors.ENDC}")
        print(f"Artifacts are in the '{build_dir_name}' directory.")
        
        if self.options["package"] and not self.options["install"]:
            print("Installation package has also been created.")
        
        print("-" * 60)
        print(f"Total time elapsed: {AnsiColors.BOLD}{minutes}m {seconds}s{AnsiColors.ENDC}")
        print("="*60)
