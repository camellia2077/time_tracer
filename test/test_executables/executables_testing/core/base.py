# src/tt_testing/core/base.py
import re
from pathlib import Path

# [修复] 改为相对导入
from ..conf.definitions import TestContext
from .executor import CommandExecutor
from .logger import TestLogger

class TestCounter:
    """
    跨测试模块共享的简单计数器。
    用于生成递增的日志文件前缀 (例如: 1_test_name.log, 2_next_test.log)
    """
    def __init__(self):
        self.value = 0
        
    def increment(self):
        self.value += 1
        return self.value

class BaseTester:
    """
    现在的基类只负责组装参数和协调 Executor 与 Logger。
    不再包含底层的 subprocess 调用或文件写入逻辑。
    """
    
    def __init__(self, counter: TestCounter, module_order: int, name: str, context: TestContext):
        self.ctx = context
        self.counter = counter
        self.module_name = name
        
        # 初始化组件：日志目录和执行器
        log_dir = self.ctx.py_output_base_dir / f"{module_order}_{name}"
        self.logger = TestLogger(log_dir)
        self.executor = CommandExecutor()

    def run_command_test(self, test_name: str, command_args: list, **kwargs) -> bool:
        """
        统一的测试执行入口。
        :param test_name: 测试用例名称
        :param command_args: CLI 参数列表 (不含 exe 本身)
        :param kwargs: 可选参数 (stdin_input, add_output_dir, print_stdout)
        """
        # 1. 准备数据 (计数、文件名)
        current_count = self.counter.increment()
        # 将非字母数字字符替换为下划线，用于文件名
        sanitized_name = re.sub(r'[^a-zA-Z0-9]+', '_', test_name).lower()
        log_file = f"{current_count}_{sanitized_name}.log"
        
        # 2. 构建完整命令
        full_cmd = [str(self.ctx.exe_path)] + command_args
        
        # 默认自动追加 --output 参数 (除非显式禁用)
        if kwargs.get('add_output_dir', True):
             full_cmd.extend(["--output", str(self.ctx.output_dir / "exported_files")])

        # 3. 执行 (委托给 Executor)
        # [修改] 关键点：设置 cwd 为 exe 所在的目录。
        # 这确保了 exe 能找到同目录下的 config/ 文件夹和 DLL 文件。
        cwd_path = self.ctx.exe_path.parent

        result = self.executor.run(
            full_cmd, 
            input_str=kwargs.get('stdin_input'),
            cwd=cwd_path 
        )

        # 4. 记录 (委托给 Logger)
        self.logger.log_result(
            test_name, 
            log_file, 
            result, 
            print_stdout=kwargs.get('print_stdout', False)
        )

        return result.return_code == 0