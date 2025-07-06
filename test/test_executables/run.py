import os
import re
import sys
import subprocess
from pathlib import Path
import shutil
import json

# 所有配置项现在从 config.json 读取

def load_config(config_path="config.json"):
    """从 JSON 文件加载配置。"""
    with open(config_path, 'r', encoding='utf-8') as f:
        return json.load(f)

def validate_config(config: dict):
    """
    新增：验证加载的配置字典是否包含所有必需的键。
    如果缺少任何键，则打印错误并退出程序。
    """
    print("--- 2. 验证配置文件 'config.json' ---")
    try:
        # 定义必需的键结构
        required_keys = {
            "shared_paths": ["executable", "source_data", "processed_data"],
            "query_module": ["daily_query_date", "period_query_days", "monthly_query_month"]
        }

        # 逐层检查
        for top_key, sub_keys in required_keys.items():
            if top_key not in config:
                print(f"错误：'config.json' 文件中缺少顶级配置项: '{top_key}'")
                sys.exit(1)
            for sub_key in sub_keys:
                if sub_key not in config[top_key]:
                    print(f"错误：'config.json' 的 '{top_key}' 中缺少配置项: '{sub_key}'")
                    sys.exit(1)
                    
    except TypeError:
        print("错误：'config.json' 文件格式不正确，无法解析。")
        sys.exit(1)
        
    print("✔️ 配置文件 'config.json' 内容完整。")


def strip_ansi_codes(text: str) -> str:
    """使用正则表达式移除文本中的所有ANSI转义码（颜色代码）。"""
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
    测试器基类，现在从配置字典中初始化属性。
    """
    def __init__(self, counter: TestCounter, module_order: int, reports_sub_dir_name: str, config: dict):
        paths = config['shared_paths']
        self.executable_path = Path(paths['executable'])
        self.source_path = Path(paths['source_data'])
        self.expected_output_path = Path(paths['processed_data'])
        self.test_counter = counter
        self.config = config 
        
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
        if command_args[0] == '-q':
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
                    if self.expected_output_path.exists():
                        log_all(f"    ✔️ 成功！预期的输出路径 '{self.expected_output_path.name}' 已创建。")
                    else:
                        log_all(f"    ❌ 失败！预期的输出路径 '{self.expected_output_path.name}' 未被创建。")
            except Exception as e:
                log_all(f"  执行命令时发生未知错误: {e}")
            log_all(outer_separator + "\n")

class PreprocessingTester(BaseTester):
    """文件预处理测试类。"""
    def __init__(self, counter: TestCounter, module_order: int, config: dict):
        super().__init__(counter, module_order, "reprocessing", config)

    def run_tests(self):
        """运行所有与 Pre-processing 相关的测试。"""
        self._log_to_console(f"\n--- [模块开始]: 文件预处理测试 (结果 -> {self.reports_dir.relative_to(Path.cwd())}) ---\n")
        self.run_command_test("Pre-processing 命令测试 -c", ["-c", str(self.source_path)])
        self.run_command_test("Pre-processing 命令测试 -vs", ["-vs", str(self.source_path)])
        self.run_command_test("Pre-processing 命令测试 -c -vo", ["-c", "-vo", str(self.source_path)])
        self.run_command_test("Pre-processing 命令测试 -c -vo -edc", ["-c", "-vo", "-edc", str(self.source_path)])

class DatabaseImportTester(BaseTester):
    """数据库导入测试类。"""
    def __init__(self, counter: TestCounter, module_order: int, config: dict):
        super().__init__(counter, module_order, "db_import", config)

    def run_tests(self):
        self._log_to_console(f"\n--- [模块开始]: 数据库导入测试 (结果 -> {self.reports_dir.relative_to(Path.cwd())}) ---\n")
        if not self.expected_output_path.exists():
            self._log_to_console(f"警告：跳过数据库导入测试，因为预期的输入目录 '{self.expected_output_path.name}' 不存在。")
            return
        self.run_command_test("Database Import 命令测试 -p", ["-p", str(self.expected_output_path)], stdin_input="y\n")

class QueryTester(BaseTester):
    """数据查询测试类，从配置中读取查询参数。"""
    def __init__(self, counter: TestCounter, module_order: int, config: dict):
        super().__init__(counter, module_order, "query", config)
        query_params = self.config['query_module']
        self.query_date = query_params['daily_query_date']
        self.query_days = query_params['period_query_days']
        self.query_month = query_params['monthly_query_month']

    def run_tests(self):
        self._log_to_console(f"\n--- [模块开始]: 数据查询测试 (结果 -> {self.reports_dir.relative_to(Path.cwd())}) ---\n")
        db_file = Path("./time_data.db")
        if not db_file.exists():
            self._log_to_console(f"警告：跳过数据查询测试，因为数据库文件 '{db_file.name}' 不存在。")
            return
        self.run_command_test("Data Query 命令测试 -q d", ["-q", "d", self.query_date])
        self.run_command_test("Data Query 命令测试 -q p", ["-q", "p", self.query_days])
        self.run_command_test("Data Query 命令测试 -q m", ["-q", "m", self.query_month])

def setup_environment(config: dict):
    """
    执行所有测试前的一次性全局环境设置和清理。
    """
    print("--- 1. 验证测试环境依赖路径 ---")
    paths = config['shared_paths']
    executable_path = Path(paths['executable'])
    source_path = Path(paths['source_data'])
    expected_output_path = Path(paths['processed_data'])

    all_ok = True
    if not executable_path.exists():
        print(f"错误：可执行文件未找到: {executable_path}")
        all_ok = False
    else:
        print(f"✔️ 可执行文件找到: {executable_path}")
    if not source_path.exists():
        print(f"错误：源文件/目录未找到: {source_path}")
        all_ok = False
    else:
        print(f"✔️ 源文件/目录找到: {source_path}")
    if not all_ok:
        print("\n环境检查失败，请修正 'config.json' 中的路径后重试。")
        sys.exit(1)
    
    output_dir = Path.cwd() / "output"
    if not output_dir.exists():
        print("✔️ 'output' 目录不存在，环境干净。")

    if expected_output_path.exists():
        print(f"警告：发现旧的转换后数据路径，将删除: {expected_output_path}")
        shutil.rmtree(expected_output_path)
    
    db_file = Path("./time_data.db")
    if db_file.exists():
        print(f"警告：发现旧的数据库文件，将删除: {db_file.name}")
        db_file.unlink()
    
    print("✔️ 环境验证和清理完成，准备开始测试。")

def main():
    """
    主测试函数，现在首先加载并验证配置，然后将其传递给所有需要它的部分。
    """
    try:
        config = load_config()
    except FileNotFoundError:
        # 修改点：更明确的错误提示
        print("错误：配置文件 'config.json' 未找到。")
        print("请确保 'config.json' 文件与 'run.py' 在同一目录下，并包含所有必要的配置项。")
        sys.exit(1)

    # 修改点：在执行任何操作前，先验证配置文件的内容
    validate_config(config)

    setup_environment(config)
    
    shared_counter = TestCounter()
    
    # 模块 1: 文件预处理
    preproc_tester = PreprocessingTester(shared_counter, 1, config)
    preproc_tester.reports_dir.mkdir(parents=True, exist_ok=True) 
    preproc_tester.run_tests()
    
    # 模块 2: 数据库导入
    db_import_tester = DatabaseImportTester(shared_counter, 2, config)
    db_import_tester.reports_dir.mkdir(parents=True, exist_ok=True)
    db_import_tester.run_tests()
    
    # 模块 3: 数据查询
    query_tester = QueryTester(shared_counter, 3, config)
    query_tester.reports_dir.mkdir(parents=True, exist_ok=True)
    query_tester.run_tests()
    
    output_dir = Path.cwd() / "output"
    final_message_1 = "--- 所有测试执行完毕 ---"
    final_message_2 = f"所有测试日志已按模块保存在文件夹中: {output_dir.resolve()}"
    
    print("\n" + "="*80)
    print(final_message_1)
    print(final_message_2)
    print("="*80)

if __name__ == "__main__":
    main()