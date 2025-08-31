# test_exe_main.py
import sys
import shutil
import os
from pathlib import Path

# --- 从配置文件导入所有配置 ---
import config

# --- 内部测试模块 ---
from _py_internal.base_module import TestCounter
from _py_internal.module_preprocessing import PreprocessingTester
from _py_internal.module_database import DatabaseImportTester
from _py_internal.module_query import QueryTester
from _py_internal.module_export import ExportTester


def setup_environment():
    """验证路径、清理旧环境，然后复制可执行文件、DLL和配置。"""
    
    # ... (此函数内容保持不变) ...
    # --- 第1步: 清理上一次运行留下的所有工件 ---
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
                
    db_file = Path.cwd() / config.GENERATED_DB_FILE_NAME
    if db_file.exists():
        db_file.unlink()
        print(f"  {config.Colors.GREEN}已移除旧数据库文件: {config.GENERATED_DB_FILE_NAME}{config.Colors.RESET}")
        
    output_dir = Path.cwd() / "output"
    output_dir.mkdir(parents=True, exist_ok=True)
    print(f"  {config.Colors.GREEN}清理完成，已创建 'output' 日志目录。{config.Colors.RESET}")

    # --- 第2步: 准备本次运行所需的文件 ---
    print(f"{config.Colors.CYAN}--- 2. Preparing Executable, DLLs and Config ---{config.Colors.RESET}")
    
    if not config.SOURCE_EXECUTABLES_DIR.exists():
        print(f"  {config.Colors.RED}错误: 源目录不存在: {config.SOURCE_EXECUTABLES_DIR}{config.Colors.RESET}")
        sys.exit(1)

    # 复制 EXE 和 DLL 文件
    executables_to_copy = [config.EXECUTABLE_CLI_NAME, config.EXECUTABLE_APP_NAME]
    dlls_to_copy = [
        "libgcc_s_seh-1.dll",
        "libstdc++-6.dll",
        "libsqlite3-0.dll",
        "libwinpthread-1.dll"
    ]
    artifacts_to_copy = executables_to_copy + dlls_to_copy

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
    
    # 复制 config 文件夹
    source_config_path = config.SOURCE_EXECUTABLES_DIR / "config"
    target_config_path = config.TARGET_EXECUTABLES_DIR / "config"
    if source_config_path.exists() and source_config_path.is_dir():
        try:
            shutil.copytree(source_config_path, target_config_path)
            print(f"  {config.Colors.GREEN}已成功复制: config 文件夹{config.Colors.RESET}")
        except Exception as e:
            print(f"  {config.Colors.RED}复制 config 文件夹时出错: {e}{config.Colors.RESET}")
            sys.exit(1)
    else:
        print(f"  {config.Colors.RED}警告: 在源目录中未找到 config 文件夹，跳过复制。{config.Colors.RESET}")

    print("  可执行文件、DLL和配置已准备就绪。")


def main():
    """运行所有测试模块的主函数。"""
    os.system('')
    
    print("\n" + "="*50)
    print(f" Running Python test script: {Path(__file__).name}")
    print(f" Current directory: {Path.cwd()}")
    print(f" Input data path: {config.SOURCE_DATA_PATH}")
    print(f" Expecting processed folder: {config.PROCESSED_DATA_DIR_NAME}")
    print("="*50 + "\n")
    
    setup_environment()
    
    print("\n========== Starting Test Sequence ==========")
    
    shared_counter = TestCounter()
    
    common_args = {
        "executable_to_run": config.EXECUTABLE_CLI_NAME,
        "source_data_path": config.SOURCE_DATA_PATH,
        "converted_text_dir_name": config.PROCESSED_DATA_DIR_NAME
    }

    modules = [
        PreprocessingTester(shared_counter, 1, 
                            # [核心修改] 直接使用从 config.py 导入的路径
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
    
    all_tests_passed = True
    for i, module in enumerate(modules, 1):
        module.reports_dir.mkdir(parents=True, exist_ok=True)
        print(f"{config.Colors.CYAN}--- {i}. Running {module.module_name} Tasks ---{config.Colors.RESET}")
        
        if not module.run_tests():
            all_tests_passed = False
            print(f"\n{config.Colors.RED}错误: 测试序列因 '{module.module_name}' 模块执行失败而中断。{config.Colors.RESET}")
            break

    if all_tests_passed:
        print(f"""
{config.Colors.GREEN}✅ All test steps completed successfully!{config.Colors.RESET}
   Check the 'output' directory for detailed logs.
""")

if __name__ == "__main__":
    main()