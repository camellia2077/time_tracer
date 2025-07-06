import os
import re
import sys
import subprocess
from pathlib import Path
import shutil

# --- 配置区 ---
# 请确保以下路径正确无误。使用 r"..." 格式来避免路径问题。

# 1. 您的可执行文件的完整路径
EXECUTABLE_PATH = r"C:\Computer\github_cpp\Time_Master_cpp\test\time_tracker_cli.exe"

# 2. 您的源文件或源文件夹的路径
SOURCE_PATH = r"C:\Computer\github_cpp\Time_Master_cpp\test\Date"

# 3. 您的程序转换后，预期的输出文件或文件夹路径
EXPECTED_OUTPUT_PATH = r"C:\Computer\github_cpp\Time_Master_cpp\test\Processed_Date"

# 4. 新增：存放所有独立测试报告的文件夹名称
OUTPUT_DIR_NAME = "Reprocessing"
# --- 配置区结束 ---

def strip_ansi_codes(text: str) -> str:
    """
    使用正则表达式移除文本中的所有ANSI转义码（颜色代码）。
    """
    ansi_escape_pattern = re.compile(r'\x1b\[[0-9;]*m')
    return ansi_escape_pattern.sub('', text)

class PreprocessingTest:
    def __init__(self):
        """
        初始化测试类，并创建或清空用于存放结果的 Reprocessing 文件夹。
        """
        self.executable_path = Path(EXECUTABLE_PATH)
        self.source_path = Path(SOURCE_PATH)
        self.expected_output_path = Path(EXPECTED_OUTPUT_PATH)
        self.test_counter = 0
        
        # 定义并准备输出目录
        script_dir = Path(__file__).parent
        self.output_dir = script_dir / OUTPUT_DIR_NAME
        
        # 如果目录已存在，先删除再创建，确保每次运行都是全新的结果
        if self.output_dir.exists():
            shutil.rmtree(self.output_dir)
        self.output_dir.mkdir(parents=True, exist_ok=True)


    def _log_to_console(self, message):
        """辅助方法：只打印到控制台。"""
        print(message)


    def verify_environment(self):
        """检查所需的文件和目录是否存在，只在控制台打印信息。"""
        self._log_to_console("--- 1. 验证测试环境 ---")
        all_ok = True

        if not self.executable_path.exists():
            self._log_to_console(f"错误：可执行文件未找到: {self.executable_path}")
            all_ok = False
        else:
            self._log_to_console(f"✔️ 可执行文件找到: {self.executable_path}")
            
        if not self.source_path.exists():
            self._log_to_console(f"错误：源文件/目录未找到: {self.source_path}")
            all_ok = False
        else:
            self._log_to_console(f"✔️ 源文件/目录找到: {self.source_path}")

        if not all_ok:
            self._log_to_console("\n环境检查失败，请修正配置区中的路径后重试。")
            sys.exit(1)

        if self.expected_output_path.exists():
            self._log_to_console(f"警告：发现旧的输出路径，将删除以便进行准确测试: {self.expected_output_path}")
            try:
                if self.expected_output_path.is_dir():
                    shutil.rmtree(self.expected_output_path)
                else:
                    self.expected_output_path.unlink()
            except OSError as e:
                self._log_to_console(f"删除旧输出失败: {e}")
                
        self._log_to_console("环境验证通过，准备开始测试。\n")


    def run_command_test(self, test_name: str, command_args: list):
        """
        运行单个测试，并将该测试的所有输出保存到以其命令命名的独立 .txt 文件中。
        """
        self.test_counter += 1
        
        # --- 准备日志格式 ---
        title = f" {self.test_counter}. 测试: {test_name} "
        outer_separator = "=" * 80
        inner_separator = "-" * 80
        
        # --- 修改点：根据命令参数生成新的、更简洁的文件名 ---
        # 1. 从 command_args 列表中筛选出所有以 '-' 开头的命令标志
        flags = [arg for arg in command_args if arg.startswith('-')]
        
        # 2. 将标志用下划线连接起来，例如：['-c', '-vo'] -> "_-c_-vo"
        flags_part = "".join([f"_{flag}" for flag in flags])
        
        # 3. 组合成最终的文件名，例如："4_-c_-vo_-edc.txt"
        log_filename = f"{self.test_counter}{flags_part}.txt"
        # --- 文件名修改结束 ---

        log_filepath = self.output_dir / log_filename

        # --- 打开此测试专用的日志文件 ---
        with open(log_filepath, 'w', encoding='utf-8') as log_file:
            
            # 定义一个内部函数，用于同时向控制台和当前日志文件写入信息
            def log_all(message):
                self._log_to_console(message)
                log_file.write(strip_ansi_codes(message) + '\n')
            
            # --- 开始记录并执行测试 ---
            log_all(outer_separator)
            log_all(f"=={title}".ljust(len(outer_separator) - 2) + "==")
            
            log_all(inner_separator)
            command = [str(self.executable_path)] + command_args
            log_all(f"step 1: 执行命令\n  {' '.join(command)}")
            log_all(inner_separator)

            log_all(f"step 2: 程序输出及结果分析")
            try:
                result = subprocess.run(
                    command,
                    capture_output=True,
                    text=True,
                    check=False,
                    encoding='utf-8',
                    errors='ignore'
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


    def run_all_preprocessing_tests(self):
        """运行所有与 Pre-processing 模块相关的测试。"""
        self.run_command_test("Pre-processing 命令测试 -c", ["-c", str(self.source_path)])
        self.run_command_test("Pre-processing 命令测试 -vs", ["-vs", str(self.source_path)])
        self.run_command_test("Pre-processing 命令测试 -c -vo", ["-c", "-vo", str(self.source_path)])
        self.run_command_test("Pre-processing 命令测试 -c -vo -edc", ["-c", "-vo", "-edc", str(self.source_path)])


def main():
    """主测试函数"""
    tester = PreprocessingTest()
    tester.verify_environment()
    tester.run_all_preprocessing_tests()
    
    # 最终的结束信息只打印在控制台
    final_message_1 = "--- 所有测试执行完毕 ---"
    final_message_2 = f"所有独立的测试日志已保存在文件夹中: {tester.output_dir.resolve()}"
    
    print("\n" + "="*80)
    print(final_message_1)
    print(final_message_2)
    print("="*80)


if __name__ == "__main__":
    main()