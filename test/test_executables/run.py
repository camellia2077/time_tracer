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

# 4. 测试日志输出文件名
LOG_FILE_NAME = "output.txt"
# --- 配置区结束 ---

def strip_ansi_codes(text: str) -> str:
    """
    使用正则表达式移除文本中的所有ANSI转义码（颜色代码）。
    """
    ansi_escape_pattern = re.compile(r'\x1b\[[0-9;]*m')
    return ansi_escape_pattern.sub('', text)

class PreprocessingTest:
    def __init__(self, log_file):
        self.log_file = log_file
        self.executable_path = Path(EXECUTABLE_PATH)
        self.source_path = Path(SOURCE_PATH)
        self.expected_output_path = Path(EXPECTED_OUTPUT_PATH)

    def _log(self, message, to_console=True, to_file=True, strip_ansi=False):
        """辅助方法：同时打印到控制台和写入文件。"""
        if to_console:
            print(message)
        if to_file:
            if strip_ansi:
                self.log_file.write(strip_ansi_codes(message) + '\n')
            else:
                self.log_file.write(message + '\n')
        self.log_file.flush()

    def verify_environment(self):
        """检查所需的文件和目录是否存在，并将日志写入文件。"""
        self._log("--- 1. 验证测试环境 ---")
        all_ok = True

        if not self.executable_path.exists():
            self._log(f"错误：可执行文件未找到: {self.executable_path}")
            all_ok = False
        else:
            self._log(f"✔️ 可执行文件找到: {self.executable_path}")
            
        if not self.source_path.exists():
            self._log(f"错误：源文件/目录未找到: {self.source_path}")
            all_ok = False
        else:
            self._log(f"✔️ 源文件/目录找到: {self.source_path}")

        if not all_ok:
            self._log("\n环境检查失败，请修正配置区中的路径后重试。")
            sys.exit(1)

        if self.expected_output_path.exists():
            self._log(f"警告：发现旧的输出路径，将删除以便进行准确测试: {self.expected_output_path}")
            try:
                if self.expected_output_path.is_dir():
                    shutil.rmtree(self.expected_output_path)
                else:
                    self.expected_output_path.unlink()
            except OSError as e:
                self._log(f"删除旧输出失败: {e}")
                
        self._log("环境验证通过，准备开始测试。\n")

    def run_command_test(self, test_name: str, command_args: list):
        """运行测试，将带颜色的输出打印到控制台，纯文本输出写入日志文件。"""
        
        self._log(f"--- 测试: {test_name} ---")
        
        command = [str(self.executable_path)] + command_args
        self._log(f"执行命令: {' '.join(command)}")

        try:
            result = subprocess.run(
                command,
                capture_output=True,
                text=True,
                check=False,
                encoding='utf-8',
                errors='ignore'
            )

            self._log(f"  [退出代码]: {result.returncode}")
            self._log(f"  [标准输出 (stdout)]:\n{result.stdout.strip() or '无'}", strip_ansi=True)
            self._log(f"  [标准错误 (stderr)]:\n{result.stderr.strip() or '无'}", strip_ansi=True)

            if "-c" in command_args or "--convert" in command_args:
                if self.expected_output_path.exists():
                    self._log(f"  [文件检查]: 成功！预期的输出路径 '{self.expected_output_path.name}' 已创建。")
                else:
                    self._log(f"  [文件检查]: 失败！预期的输出路径 '{self.expected_output_path.name}' 未被创建。")
            
        except Exception as e:
            self._log(f"执行命令时发生未知错误: {e}")

        self._log("-" * (len(strip_ansi_codes(test_name)) + 8) + "\n")

    def run_all_preprocessing_tests(self):
        """运行所有与 Pre-processing 模块相关的测试。"""
        self.run_command_test("Pre-processing: 命令测试 -c", ["-c", str(self.source_path)])
        self.run_command_test("Pre-processing: 命令测试 -vs", ["-vs", str(self.source_path)])
        self.run_command_test("Pre-processing: 命令测试 -c -vo", ["-c", "-vo", str(self.source_path)])
        self.run_command_test("Pre-processing: 命令测试 -c -vo -edc", ["-c", "-vo", "-edc", str(self.source_path)])


def main():
    """主测试函数"""
    script_dir = Path(__file__).parent
    log_file_path = script_dir / LOG_FILE_NAME

    # 使用 'with' 语句确保文件最终会被正确关闭
    with open(log_file_path, 'w', encoding='utf-8') as log_file:
        preprocessing_tester = PreprocessingTest(log_file)
        preprocessing_tester.verify_environment()
        preprocessing_tester.run_all_preprocessing_tests()
        
        final_message_1 = "--- 所有测试执行完毕 ---"
        final_message_2 = "测试环境和生成的文件已被保留。"
        final_message_3 = f"完整的纯文本测试日志已保存在: {log_file_path.resolve()}"
        
        print(final_message_1)
        print(final_message_2)
        print(final_message_3)
        log_file.write(f"{final_message_1}\n{final_message_2}\n{final_message_3}\n")


if __name__ == "__main__":
    main()