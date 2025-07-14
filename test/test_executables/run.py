import os
import re
import sys
import subprocess
from pathlib import Path
import shutil
from datetime import datetime

# --- 全局配置参数 ---
# 定义 time_tracker_cli.exe 的文件名。在测试开始前，此文件会被删除并从 source_executables_dir 复制到 target_executables_dir.
EXECUTABLE_CLI_NAME = "time_tracker_cli.exe" 
# 定义 time_tracker_app.exe 的文件名。在测试开始前，此文件会被删除并从 source_executables_dir 复制到 target_executables_dir.
EXECUTABLE_APP_NAME = "time_tracker_app.exe" 
# 包含 time_tracker_cli.exe 和 time_tracker_app.exe 的源目录路径.
SOURCE_EXECUTABLES_DIR = Path("C:/Computer/my_github/github_cpp/Time_Master_cpp/time_master/build") 
# 目标目录路径，可执行文件将被复制到这里。"./" 表示 run.py 脚本所在的当前目录.
TARGET_EXECUTABLES_DIR = Path("./") 

# 主要的可执行文件，测试脚本会运行这个可执行文件。这里是 time_tracker_cli.exe 的文件名，因为它会被复制到当前目录.
EXECUTABLE_TO_RUN = "time_tracker_cli.exe" 
# 包含原始源数据文件(例如 "Date_test")的目录路径.
SOURCE_DATA_PATH = Path("C:/Computer/my_github/github_cpp/Time_Master_cpp/test/Date_test")
# 转换后的处理数据将存储的目录路径(例如 "Processed_Date_test")。在每次测试前会被删除.
PROCESSED_DATA_DIR_NAME = "Processed_Date_test" # 保持名称，便于构建 Path
# 测试过程中生成的数据库文件名(例如 "time_data.db")。在每次测试前会被删除.
GENERATED_DB_FILE_NAME = "time_data.db"              
# 转换文本后生成的数据目录名，通常与 processed_data 的最后一部分相同。在每次测试前会被删除.
CONVERTED_TEXT_DIR_NAME = "Processed_Date_test"

# 查询模块参数
DAILY_QUERY_DATE = datetime.now().strftime("%Y%m%d")
PERIOD_QUERY_DAYS = "7"
MONTHLY_QUERY_MONTH = datetime.now().strftime("%Y%m")


def strip_ansi_codes(text: str) -> str:
    """使用正则表达式移除文本中的所有ANSI转义码(颜色代码)。"""
    ansi_escape_pattern = re.compile(r'\x1b\[[0-9;]*m')
    return ansi_escape_pattern.sub('', text)

class TestCounter:
    """一个简单的可变对象，用于在不同的测试器实例间共享和递增测试序号。"""
    def __init__(self):
        self.value = 0
    def increment(self):
        self.value += 1
        return self.value

class BaseTester:
    """
    测试器基类，现在从全局变量初始化属性。
    """
    def __init__(self, counter: TestCounter, module_order: int, reports_sub_dir_name: str): # 移除 config 参数
        # 直接使用全局变量
        self.executable_path = Path.cwd() / EXECUTABLE_TO_RUN 
        self.source_path = SOURCE_DATA_PATH
        self.expected_output_path = Path.cwd() / CONVERTED_TEXT_DIR_NAME
        self.test_counter = counter
        
        reports_dir_name_with_order = f"{module_order}_{reports_sub_dir_name}"
        self.reports_dir = Path.cwd() / "output" / reports_dir_name_with_order

    def _log_to_console(self, message):
        """辅助方法：只打印到控制台。"""
        print(message)

    def run_command_test(self, test_name: str, command_args: list, stdin_input: str = None):
        """通用的命令测试执行器。"""
        current_count = self.test_counter.increment()
        title = f" {current_count}. 测试: {test_name} "
        outer_separator = "=" * 80
        inner_separator = "-" * 80
        # 优化日志文件名生成逻辑，使其同样适用于 -e 命令
        if command_args[0] in ['-q', '-e'] and len(command_args) > 1:
            flags_part = f"_{command_args[0]}_{command_args[1]}"
        else:
            flags = [arg for arg in command_args if arg.startswith('-')]
            flags_part = "".join([f"_{flag}" for flag in flags])
        log_filename = f"{current_count}{flags_part}.txt"
        log_filepath = self.reports_dir / log_filename
        with open(log_filepath, 'w', encoding='utf-8') as log_file:
            def log_all(message):
                self._log_to_console(message)
                log_file.write(strip_ansi_codes(message) + '\n')
            log_all(outer_separator)
            log_all(f"=={title}".ljust(len(outer_separator) - 2) + "==")
            log_all(inner_separator)
            command = [str(self.executable_path)] + command_args
            log_all(f"step 1: 执行命令\n  {' '.join(command)}")
            log_all(inner_separator)
            log_all(f"step 2: 程序输出及结果分析")
            try:
                result = subprocess.run(
                    command, input=stdin_input, capture_output=True, text=True,
                    check=False, encoding='utf-8', errors='ignore'
                )
                log_all(f"  [退出代码]: {result.returncode}")
                log_all(f"  [程序输出 - STDOUT]:\n{result.stdout.strip() or '无'}")
                log_all(f"  [程序输出 - STDERR]:\n{result.stderr.strip() or '无'}")
                if "-c" in command_args or "--convert" in command_args:
                    log_all(f"\n  [文件检查]:")
                    # 检查 expected_output_path 是否存在时，现在使用其作为目录进行检查
                    if self.expected_output_path.exists() and self.expected_output_path.is_dir():
                        log_all(f"    ✔️ 成功！预期的输出目录 '{self.expected_output_path.name}' 已创建。")
                    else:
                        log_all(f"    ❌ 失败！预期的输出目录 '{self.expected_output_path.name}' 未被创建。")
            except Exception as e:
                log_all(f"  执行命令时发生未知错误: {e}")
            log_all(outer_separator + "\n")

class PreprocessingTester(BaseTester):
    """文件预处理测试类。"""
    def __init__(self, counter: TestCounter, module_order: int): # 移除 config 参数
        super().__init__(counter, module_order, "reprocessing") # 移除 config 传递

    def run_tests(self):
        """运行所有与 Pre-processing 相关的测试。"""
        self._log_to_console(f"\n--- [模块开始]: 文件预处理测试 (结果 -> {self.reports_dir.relative_to(Path.cwd())}) ---\n")
        self.run_command_test("Pre-processing 命令测试 -c", ["-c", str(self.source_path)])
        self.run_command_test("Pre-processing 命令测试 -vs", ["-vs", str(self.source_path)])
        self.run_command_test("Pre-processing 命令测试 -c -vo", ["-c", "-vo", str(self.source_path)])
        self.run_command_test("Pre-processing 命令测试 -c -vo -edc", ["-c", "-vo", "-edc", str(self.source_path)])

class DatabaseImportTester(BaseTester):
    """数据库导入测试类。"""
    def __init__(self, counter: TestCounter, module_order: int): # 移除 config 参数
        super().__init__(counter, module_order, "db_import") # 移除 config 传递

    def run_tests(self):
        self._log_to_console(f"\n--- [模块开始]: 数据库导入测试 (结果 -> {self.reports_dir.relative_to(Path.cwd())}) ---\n")
        # 这里的 expected_output_path 应该是转换后的数据目录，而不是数据库文件
        if not self.expected_output_path.exists():
            self._log_to_console(f"警告：跳过数据库导入测试，因为预期的输入目录 '{self.expected_output_path.name}' 不存在。")
            return
        self.run_command_test("Database Import 命令测试 -p", ["-p", str(self.expected_output_path)], stdin_input="y\n")

class QueryTester(BaseTester):
    """数据查询测试类，从全局变量读取查询参数。"""
    def __init__(self, counter: TestCounter, module_order: int): # 移除 config 参数
        super().__init__(counter, module_order, "query") # 移除 config 传递
        # 直接使用全局变量
        self.query_date = DAILY_QUERY_DATE
        self.query_days = PERIOD_QUERY_DAYS
        self.query_month = MONTHLY_QUERY_MONTH

    def run_tests(self):
        self._log_to_console(f"\n--- [模块开始]: 数据查询测试 (结果 -> {self.reports_dir.relative_to(Path.cwd())}) ---\n")
        # db_file 的路径现在从全局变量获取
        db_file = Path.cwd() / GENERATED_DB_FILE_NAME 
        if not db_file.exists():
            self._log_to_console(f"警告：跳过数据查询测试，因为数据库文件 '{db_file.name}' 不存在。")
            return
        self.run_command_test("Data Query 命令测试 -q d", ["-q", "d", self.query_date])
        self.run_command_test("Data Query 命令测试 -q p", ["-q", "p", self.query_days])
        self.run_command_test("Data Query 命令测试 -q m", ["-q", "m", self.query_month])

class ExportTester(BaseTester):
    """数据导出测试类。"""
    def __init__(self, counter: TestCounter, module_order: int):
        super().__init__(counter, module_order, "export")

    def run_tests(self):
        self._log_to_console(f"\n--- [模块开始]: 数据导出测试 (结果 -> {self.reports_dir.relative_to(Path.cwd())}) ---\n")
        
        # 导出前先检测程序同目录是否有Export文件夹，如果有先删除
        export_dir = Path.cwd() / "Export"
        if export_dir.exists():
            self._log_to_console(f"警告：发现旧的导出目录，将删除: {export_dir}")
            try:
                shutil.rmtree(export_dir)
                self._log_to_console(f"    ✔️ 已删除旧目录: {export_dir}")
            except OSError as e:
                self._log_to_console(f"    ❌ 无法删除目录 {export_dir}: {e}")
                return # 如果无法删除，则终止此模块测试

        # 检查数据库文件是否存在
        db_file = Path.cwd() / GENERATED_DB_FILE_NAME
        if not db_file.exists():
            self._log_to_console(f"警告：跳过数据导出测试，因为数据库文件 '{db_file.name}' 不存在。")
            return

        # 执行导出命令
        self.run_command_test("Data Export 命令测试 -e d", ["-e", "d"])
        self.run_command_test("Data Export 命令测试 -e m", ["-e", "m"])
        self.run_command_test("Data Export 命令测试 -e p", ["-e", "p", "7,30,90"])

        # 检查导出目录是否已创建
        self._log_to_console(f"\n  [文件检查]:")
        if export_dir.exists() and export_dir.is_dir():
            self._log_to_console(f"    ✔️ 成功！预期的输出目录 '{export_dir.name}' 已创建。")
        else:
            self._log_to_console(f"    ❌ 失败！预期的输出目录 '{export_dir.name}' 未被创建。")


def setup_environment(): 
    """
    执行所有测试前的一次性全局环境设置和清理。
    """
    print("--- 1. 验证测试环境依赖路径 ---")
    
    # 直接使用全局变量
    executable_cli_name = EXECUTABLE_CLI_NAME
    executable_app_name = EXECUTABLE_APP_NAME
    source_executables_dir = SOURCE_EXECUTABLES_DIR
    target_executables_dir = TARGET_EXECUTABLES_DIR

    executable_cli_target_path = target_executables_dir / executable_cli_name
    executable_app_target_path = target_executables_dir / executable_app_name

    source_path = SOURCE_DATA_PATH
    generated_db_file = Path.cwd() / GENERATED_DB_FILE_NAME
    converted_text_dir = Path.cwd() / CONVERTED_TEXT_DIR_NAME

    all_ok = True
    # 验证源可执行文件目录是否存在
    if not source_executables_dir.exists() or not source_executables_dir.is_dir():
        print(f"错误：源可执行文件目录未找到或不是目录: {source_executables_dir}")
        all_ok = False
    else:
        print(f"✔️ 源可执行文件目录找到: {source_executables_dir}")

    # 验证源数据目录是否存在
    if not source_path.exists():
        print(f"错误：源文件/目录未找到: {source_path}")
        all_ok = False
    else:
        print(f"✔️ 源文件/目录找到: {source_path}")

    if not all_ok:
        print("\n环境检查失败，请修正脚本中的路径后重试。")
        sys.exit(1)
    
    output_dir = Path.cwd() / "output"
    if not output_dir.exists():
        print("✔️ 'output' 目录不存在，环境干净。")

    # 删除旧的转换后文本目录
    if converted_text_dir.exists():
        print(f"警告：发现旧的转换后数据目录，将删除: {converted_text_dir}")
        try:
            shutil.rmtree(converted_text_dir)
            print(f"    ✔️ 已删除旧目录: {converted_text_dir}")
        except OSError as e:
            print(f"    ❌ 无法删除目录 {converted_text_dir}: {e}")
            sys.exit(1)
    
    # 删除旧的数据库文件
    if generated_db_file.exists():
        print(f"警告：发现旧的数据库文件，将删除: {generated_db_file.name}")
        try:
            generated_db_file.unlink()
            print(f"    ✔️ 已删除旧文件: {generated_db_file.name}")
        except OSError as e:
            print(f"    ❌ 无法删除文件 {generated_db_file.name}: {e}")
            sys.exit(1)

    # 删除并复制可执行文件
    print("\n--- 清理和复制可执行文件 ---")
    executables_to_handle = {
        executable_cli_name: executable_cli_target_path,
        executable_app_name: executable_app_target_path
    }

    for name, target_path in executables_to_handle.items():
        if target_path.exists():
            print(f"警告：发现旧的可执行文件，将删除: {target_path.name}")
            try:
                target_path.unlink()
                print(f"    ✔️ 已删除旧文件: {target_path.name}")
            except OSError as e:
                print(f"    ❌ 无法删除文件 {target_path.name}: {e}")
                sys.exit(1)

        source_path_full = source_executables_dir / name
        if source_path_full.exists():
            print(f"复制 '{name}' 从 '{source_executables_dir}' 到 '{target_executables_dir}'")
            try:
                shutil.copy2(source_path_full, target_path)
                print(f"    ✔️ 成功复制 '{name}'")
            except shutil.Error as e:
                print(f"    ❌ 复制 '{name}' 失败: {e}")
                sys.exit(1)
        else:
            print(f"错误：源可执行文件 '{name}' 未找到于 '{source_executables_dir}'")
            sys.exit(1)
            
    print("✔️ 环境验证和清理完成，准备开始测试。")

def main():
    """
    主测试函数。
    """
    setup_environment() # 直接调用，不再传递 config

    shared_counter = TestCounter()
    
    # 模块 1: 文件预处理
    # 构造函数不再需要 config 参数
    preproc_tester = PreprocessingTester(shared_counter, 1)
    preproc_tester.reports_dir.mkdir(parents=True, exist_ok=True)
    preproc_tester.run_tests()
    
    # 模块 2: 数据库导入
    # 构造函数不再需要 config 参数
    db_import_tester = DatabaseImportTester(shared_counter, 2)
    db_import_tester.reports_dir.mkdir(parents=True, exist_ok=True)
    db_import_tester.run_tests()
    
    # 模块 3: 数据查询
    # 构造函数不再需要 config 参数
    query_tester = QueryTester(shared_counter, 3)
    query_tester.reports_dir.mkdir(parents=True, exist_ok=True)
    query_tester.run_tests()
    
    # 模块 4: 数据导出
    export_tester = ExportTester(shared_counter, 4)
    export_tester.reports_dir.mkdir(parents=True, exist_ok=True)
    export_tester.run_tests()

    output_dir = Path.cwd() / "output"
    final_message_1 = "--- 所有测试执行完毕 ---"
    final_message_2 = f"所有测试日志已按模块保存在文件夹中: {output_dir.resolve()}"
    
    print("\n" + "="*80)
    print(final_message_1)
    print(final_message_2)
    print("="*80)

if __name__ == "__main__":
    main()