# test_exe_main.py
import sys
import shutil
import os
from pathlib import Path
import time
from typing import List

# --- 从配置文件导入所有配置 ---
import config

# --- 内部测试模块 ---
from _py_internal.base_module import BaseTester, TestCounter
from _py_internal.module_preprocessing import PreprocessingTester
from _py_internal.module_database import DatabaseImportTester
from _py_internal.module_query import QueryTester
from _py_internal.module_export import ExportTester


def setup_environment():
    """验证路径、清理旧环境，然后复制可执行文件、DLL和配置。"""
    
    print(f"{config.Colors.CYAN}--- 1. Cleaning Artifacts & Setting up Directories ---{config.Colors.RESET}")
    for dir_name in config.DIRECTORIES_TO_CLEAN:
        dir_path = Path.cwd() / dir_name
        if dir_path.exists():
            try:
                shutil.rmtree(dir_path)
                print(f"  {config.Colors.GREEN}已移除旧目录: {dir_name}{config.Colors.RESET}")
            except OSError as e:
                print(f"  {config.Colors.RED}移除目录 '{dir_name}' 时出错: {e}{config.Colors.RESET}")
                sys.exit(1)

    for file_name in config.FILES_TO_CLEAN:
        file_path = config.TARGET_EXECUTABLES_DIR / file_name
        if file_path.exists():
            try:
                file_path.unlink()
                print(f"  {config.Colors.GREEN}已移除旧文件: {file_name}{config.Colors.RESET}")
            except OSError as e:
                print(f"  {config.Colors.RED}移除文件 '{file_name}' 时出错: {e}{config.Colors.RESET}")
                sys.exit(1)
            
    (Path.cwd() / config.OUTPUT_DIR_NAME).mkdir(parents=True, exist_ok=True)
    (Path.cwd() / "py_output").mkdir(parents=True, exist_ok=True)
    print(f"  {config.Colors.GREEN}清理完成，已创建 'output' 和 'py_output' 目录。{config.Colors.RESET}")

    print(f"{config.Colors.CYAN}--- 2. Preparing Executable, DLLs and Config ---{config.Colors.RESET}")
    
    if not config.SOURCE_EXECUTABLES_DIR.exists():
        print(f"  {config.Colors.RED}错误: 源目录不存在: {config.SOURCE_EXECUTABLES_DIR}{config.Colors.RESET}")
        sys.exit(1)

    artifacts_to_copy = config.FILES_TO_CLEAN
    for artifact_name in artifacts_to_copy:
        source_path = config.SOURCE_EXECUTABLES_DIR / artifact_name
        target_path = config.TARGET_EXECUTABLES_DIR / artifact_name
        if not source_path.exists():
            print(f"  {config.Colors.RED}警告: 在源目录中未找到文件: {artifact_name}{config.Colors.RESET}")
            continue
        try:
            shutil.copy(source_path, target_path)
            print(f"  {config.Colors.GREEN}已成功复制: {artifact_name}{config.Colors.RESET}")
        except Exception as e:
            print(f"  {config.Colors.RED}复制文件时出错 {artifact_name}: {e}{config.Colors.RESET}")
            sys.exit(1)
    
    source_config_path = config.SOURCE_EXECUTABLES_DIR / "config"
    target_config_path = config.TARGET_EXECUTABLES_DIR / "config"
    if source_config_path.exists() and source_config_path.is_dir():
        try:
            if target_config_path.exists():
                shutil.rmtree(target_config_path)
            shutil.copytree(source_config_path, target_config_path)
            print(f"  {config.Colors.GREEN}已成功复制: config 文件夹{config.Colors.RESET}")
        except Exception as e:
            print(f"  {config.Colors.RED}复制 config 文件夹时出错: {e}{config.Colors.RESET}")
            sys.exit(1)
    else:
        print(f"  {config.Colors.RED}警告: 在源目录中未找到 config 文件夹，跳过复制。{config.Colors.RESET}")

    print("  可执行文件、DLL和配置已准备就绪。")


def print_header():
    """打印脚本的初始头部信息。"""
    print("\n" + "="*50)
    print(f" Running Python test script: {Path(__file__).name}")
    print(f" Current directory: {Path.cwd()}")
    print(f" Input data path: {config.SOURCE_DATA_PATH}")
    print(f" Expecting processed folder: {config.PROCESSED_DATA_DIR_NAME}")
    print("="*50 + "\n")


def initialize_test_modules() -> List[BaseTester]:
    """初始化并返回所有测试模块的列表。"""
    shared_counter = TestCounter()
    output_dir_path = Path.cwd() / config.OUTPUT_DIR_NAME
    
    common_args = {
        "executable_to_run": config.EXECUTABLE_CLI_NAME,
        "source_data_path": config.SOURCE_DATA_PATH,
        "converted_text_dir_name": config.PROCESSED_DATA_DIR_NAME,
        "output_dir": output_dir_path
    }

    modules = [
        PreprocessingTester(shared_counter, 1, 
                            specific_validation_path=str(config.PROCESSED_JSON_PATH),
                            **common_args),
        DatabaseImportTester(shared_counter, 2, **common_args),
        QueryTester(shared_counter, 3, 
                    generated_db_file_name=config.GENERATED_DB_FILE_NAME, 
                    daily_query_dates=config.DAILY_QUERY_DATES, 
                    monthly_query_months=config.MONTHLY_QUERY_MONTHS, 
                    period_query_days=config.PERIOD_QUERY_DAYS,
                    test_formats=config.TEST_FORMATS,
                    **common_args),
        ExportTester(shared_counter, 4, 
                     generated_db_file_name=config.GENERATED_DB_FILE_NAME,
                     is_bulk_mode=config.EXPORT_MODE_IS_BULK,
                     specific_dates=config.SPECIFIC_EXPORT_DATES,
                     specific_months=config.SPECIFIC_EXPORT_MONTHS,
                     period_export_days=config.PERIOD_EXPORT_DAYS,
                     test_formats=config.TEST_FORMATS,
                     **common_args)
    ]
    return modules


def run_test_suite(modules: List[BaseTester]) -> bool:
    """按顺序运行所提供的测试模块列表中的所有测试。"""
    print("\n========== Starting Test Sequence ==========")
    all_tests_passed = True
    for i, module in enumerate(modules, 1):
        module.reports_dir.mkdir(parents=True, exist_ok=True)
        print(f"{config.Colors.CYAN}--- {i}. Running {module.module_name} Tasks ---{config.Colors.RESET}")
        
        if not module.run_tests():
            all_tests_passed = False
            print(f"\n{config.Colors.RED}错误: 测试序列因 '{module.module_name}' 模块执行失败而中断。{config.Colors.RESET}")
            break
    return all_tests_passed


def print_summary(all_tests_passed: bool, total_duration: float):
    """根据测试结果打印最终摘要信息和总耗时。"""
    if all_tests_passed:
        print(f"""
{config.Colors.GREEN}✅ All test steps completed successfully!{config.Colors.RESET}
   Check the 'py_output' directory for detailed logs.
   Check the '{config.OUTPUT_DIR_NAME}' directory for program artifacts.
""")
    
    print(f"\n{config.Colors.CYAN}Total execution time: {total_duration:.2f} seconds.{config.Colors.RESET}")


def main():
    """运行所有测试模块的主函数。"""
    start_time = time.monotonic()
    os.system('')  # 为Windows终端初始化颜色支持
    
    print_header()
    setup_environment()
    
    test_modules = initialize_test_modules()
    all_tests_passed = run_test_suite(test_modules)
    
    total_duration = time.monotonic() - start_time
    print_summary(all_tests_passed, total_duration)


if __name__ == "__main__":
    main()