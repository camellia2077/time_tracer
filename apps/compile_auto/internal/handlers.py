# internal/handlers.py (重构后)
import os
import time
import argparse
import shutil
from abc import ABC, abstractmethod
from typing import Callable, List, Tuple, Dict, Any

from .core import process_directory, process_directory_md_via_typ
from .compilers import build_tex_command, build_typ_command, PandocCommandBuilder

# --- 通用辅助函数 ---
def format_time(seconds):
    """将秒数格式化为 HH:MM:SS """
    seconds = int(seconds)
    hours = seconds // 3600
    minutes = (seconds % 3600) // 60
    seconds = seconds % 60
    return f"{hours:02d}:{minutes:02d}:{seconds:02d}"

# --- 处理器基类 (策略模式) ---
class BaseCompilationHandler(ABC):
    """
    编译处理器的抽象基类。
    定义了所有处理器通用的执行流程和接口。
    """
    def __init__(self, args: argparse.Namespace):
        self.args = args
        self.log_name: str = "Unknown"
        self.file_extension: str = ""

    @abstractmethod
    def run(self) -> Tuple[int, int, float, List[str]]:
        """执行编译处理的核心方法。"""
        pass

    def _print_completion_message(self, success_count: int, failure_count: int):
        """打印任务完成信息。"""
        if (success_count + failure_count) > 0:
            print(f"===== {self.log_name} 处理完成 (成功: {success_count}, 失败: {failure_count}) =====")

# --- 具体的处理器实现 ---
class TeXHandler(BaseCompilationHandler):
    """处理 TeX 文件的编译，并包含特定的清理逻辑。"""
    def __init__(self, args):
        super().__init__(args)
        self.log_name = "TeX"
        self.file_extension = ".tex"

    def _cleanup_temp_files(self, directory: str):
        """在指定目录中查找并删除 .aux, .log, .out 文件。"""
        extensions_to_clean = ['.aux', '.log', '.out']
        print(f"\n--- 在 '{directory}' 中清理 TeX 临时文件 ---")
        deleted_count = 0
        for root, _, files in os.walk(directory):
            for file in files:
                if any(file.endswith(ext) for ext in extensions_to_clean):
                    path = os.path.join(root, file)
                    try:
                        os.remove(path)
                        deleted_count += 1
                    except OSError as e:
                        print(f"❌ 错误：无法删除文件 '{path}': {e}")
        if deleted_count > 0:
            print(f"--- 清理完成，共删除 {deleted_count} 个文件 ---")

    def run(self) -> Tuple[int, int, float, List[str]]:
        s, f, d, u = process_directory(
            source_dir=self.args.source_dir,
            base_output_dir=self.args.output_dir,
            file_extension=self.file_extension,
            log_file_type=self.log_name,
            command_builder=build_tex_command,
            max_workers=self.args.jobs,
            post_process_hook=self._cleanup_temp_files,
            incremental=self.args.incremental
        )
        self._print_completion_message(s, f)
        return s, f, d, u

class RSTHandler(BaseCompilationHandler):
    """处理 RST 文件的编译。"""
    def __init__(self, args):
        super().__init__(args)
        self.log_name = "RST"
        self.file_extension = ".rst"

    def run(self) -> Tuple[int, int, float, List[str]]:
        print(f"将使用字体: '{self.args.font}'")
        builder = PandocCommandBuilder(source_format='rst', font=self.args.font)
        s, f, d, u = process_directory(
            source_dir=self.args.source_dir,
            base_output_dir=self.args.output_dir,
            file_extension=self.file_extension,
            log_file_type=self.log_name,
            command_builder=builder,
            max_workers=self.args.jobs,
            incremental=self.args.incremental
        )
        self._print_completion_message(s, f)
        return s, f, d, u

class TypstHandler(BaseCompilationHandler):
    """处理 Typst 文件的编译。"""
    def __init__(self, args):
        super().__init__(args)
        self.log_name = "Typst"
        self.file_extension = ".typ"
    
    def run(self) -> Tuple[int, int, float, List[str]]:
        s, f, d, u = process_directory(
            source_dir=self.args.source_dir,
            base_output_dir=self.args.output_dir,
            file_extension=self.file_extension,
            log_file_type=self.log_name,
            command_builder=build_typ_command,
            max_workers=self.args.jobs,
            incremental=self.args.incremental
        )
        self._print_completion_message(s, f)
        return s, f, d, u

class MarkdownHandler(BaseCompilationHandler):
    """处理 Markdown 文件，支持常规编译和基准测试模式。"""
    def __init__(self, args):
        super().__init__(args)
        self.log_name = "Markdown"

    def run(self) -> Tuple[int, int, float, List[str]]:
        compilers = getattr(self.args, 'markdown_compilers', ['pandoc'])
        if len(compilers) > 1 and self.log_name.lower() in [t.lower() for t in self.args.compile_types]:
            return self._run_benchmark()
        else:
            return self._run_single_compiler()

    def _run_single_compiler(self) -> Tuple[int, int, float, List[str]]:
        compiler = self.args.markdown_compilers[0] if self.args.markdown_compilers else 'pandoc'
        print(f"===== 开始处理 Markdown (使用 {compiler} 方式) =====")
        print(f"将使用字体: '{self.args.font}'")

        if compiler == 'typst':
            results, duration, updated_files = process_directory_md_via_typ(
                source_dir=self.args.source_dir, base_output_dir=self.args.output_dir, 
                font=self.args.font, max_workers=self.args.jobs,
                incremental=self.args.incremental
            )
            success_count = sum(1 for r in results if r.get("success") and not r.get("skipped"))
            skipped_count = sum(1 for r in results if r.get("skipped"))
            failure_count = len(results) - success_count - skipped_count
            
            if len(results) > 0:
                print(f"\n--- Markdown (Typst 路径) 详细统计 ---")
                print(f"成功: {success_count}, 失败: {failure_count}, 跳过: {skipped_count}")

        else: # pandoc
            builder = PandocCommandBuilder(source_format='gfm', font=self.args.font)
            success_count, failure_count, duration, updated_files = process_directory(
                source_dir=self.args.source_dir, base_output_dir=self.args.output_dir,
                file_extension='.md', log_file_type='Markdown',
                command_builder=builder, max_workers=self.args.jobs,
                incremental=self.args.incremental
            )
        
        if (success_count + failure_count) > 0:
            print(f"===== Markdown ({compiler}) 处理完成 (成功: {success_count}, 失败: {failure_count}) =====")
        return success_count, failure_count, duration, updated_files

    def _run_benchmark(self) -> Tuple[int, int, float, List[str]]:
        print("\n" + "="*50)
        print(f"🚀  启动 Markdown 编译基准测试模式  🚀")
        print(f"   比较方法: {', '.join(self.args.markdown_compilers)}")
        print(f"   循环次数: {self.args.benchmark_loops} 次")
        print(f"   使用字体: '{self.args.font}'")
        print("="*50)

        benchmark_results: Dict[str, List[float]] = {c: [] for c in self.args.markdown_compilers}
        
        for i in range(self.args.benchmark_loops):
            print(f"\n--- 第 {i + 1}/{self.args.benchmark_loops} 轮测试 ---")
            for compiler in self.args.markdown_compilers:
                # 清理之前的输出以保证测试准确性
                target_output_path = os.path.join(self.args.output_dir, os.path.basename(self.args.source_dir))
                if os.path.exists(target_output_path):
                    shutil.rmtree(target_output_path)
                
                print(f"  > 正在测试: {compiler}...")
                duration = 0.0
                if compiler == 'pandoc':
                    builder = PandocCommandBuilder(source_format='gfm', font=self.args.font)
                    _, _, duration, _ = process_directory(self.args.source_dir, self.args.output_dir, '.md', 'Markdown', builder, self.args.jobs, quiet=True, incremental=False)
                elif compiler == 'typst':
                    _, duration, _ = process_directory_md_via_typ(self.args.source_dir, self.args.output_dir, font=self.args.font, max_workers=self.args.jobs, quiet=True, incremental=False)
                
                benchmark_results[compiler].append(duration)
                print(f"    本轮耗时: {duration:.4f} 秒")

        self._print_benchmark_summary(benchmark_results)
        return 1, 0, sum(sum(v) for v in benchmark_results.values()), []

    def _print_benchmark_summary(self, results: Dict[str, List[float]]):
        print("\n" + "="*50 + "\n📊  基准测试结果摘要  📊\n" + "="*50)
        total_times = {c: sum(d) for c, d in results.items()}
        for compiler, total_time in total_times.items():
            avg_time = total_time / len(results[compiler])
            print(f"方法: {compiler.upper()}\n  - 总耗时: {total_time:.4f} 秒\n  - 平均耗时: {avg_time:.4f} 秒/轮")
        if len(total_times) > 1:
            best_compiler = min(total_times, key=total_times.get)
            print("-" * 50 + f"\n🏆 结论: [{best_compiler.upper()}] 性能更优！")
        print("="*50)


# --- 主调度逻辑 (handle_auto) ---
def handle_auto(args: argparse.Namespace):
    """
    自动发现并执行编译任务的主调度函数。
    【重构后】使用基于类的处理器映射表。
    """
    parent_dir = args.source_dir
    print(f"===== 启动自动编译模式 =====")
    print(f"扫描父目录: '{parent_dir}'")

    # 【核心修改】将函数映射改为类映射
    compiler_map: dict[Tuple[str, ...], Tuple[str, type[BaseCompilationHandler]]] = {
        ('latex', 'tex'): ('TeX', TeXHandler),
        ('markdown', 'md'): ('Markdown', MarkdownHandler),
        ('rst', 'rest'): ('RST', RSTHandler),
        ('typst', 'typ'): ('Typst', TypstHandler)
    }

    tasks_to_run = _discover_tasks(parent_dir, compiler_map, args.compile_types)
    if not tasks_to_run:
        print(f"\n在 '{parent_dir}' 中没有找到任何需要编译的目录。 (配置类型: {args.compile_types})")
        return
        
    time_summary, stats_summary, update_summary = _execute_tasks(tasks_to_run, args)
    
    # 打印各种摘要信息的逻辑保持不变
    if time_summary:
        _print_time_summary(time_summary)
    if stats_summary:
        _print_stats_summary(stats_summary)
    if update_summary:
        _print_update_summary(update_summary)


def _discover_tasks(source_dir: str, compiler_map: Dict, types_to_compile: List[str]) -> List[Dict[str, Any]]:
    """发现任务，逻辑不变。"""
    tasks = []
    types_to_process_lower = [t.lower() for t in types_to_compile]
    print(f"注意：根据配置，将只编译以下类型 -> {types_to_compile}")
    for subdir_name in os.listdir(source_dir):
        full_subdir_path = os.path.join(source_dir, subdir_name)
        if not os.path.isdir(full_subdir_path): continue
        
        base_name_to_match = subdir_name.split('_')[0].lower()
        for keywords, (log_name, handler_class) in compiler_map.items():
            if base_name_to_match in keywords:
                if log_name.lower() in types_to_process_lower:
                    print(f"\n>>> 检测到 '{subdir_name}' -> 将使用 {log_name} 编译器...")
                    # 【核心修改】存储的是处理器类，而不是函数
                    tasks.append({'log_name': log_name, 'handler_class': handler_class, 'source_path': full_subdir_path})
                else:
                    print(f"\n>>> 检测到 '{subdir_name}' -> 类型 '{log_name}' 不在编译列表中，已跳过。")
                break
    return tasks


def _execute_tasks(tasks: List[Dict[str, Any]], args: argparse.Namespace) -> Tuple[Dict, Dict, Dict]:
    """
    执行所有任务。
    【重构后】动态创建和调用处理器实例。
    """
    timing_summary = {}
    compilation_stats = {}
    update_summary: Dict[str, int] = {}
    
    for task in tasks:
        # 创建一个新的参数副本，以防修改影响其他任务
        task_args = argparse.Namespace(**vars(args))
        task_args.source_dir = task['source_path']
        
        # 【核心修改】动态创建处理器实例并运行
        handler_instance = task['handler_class'](task_args)
        success_count, failure_count, duration, updated_files = handler_instance.run()
        
        # 结果记录逻辑不变
        if duration > 0:
            timing_summary[task['log_name']] = (duration, success_count + failure_count)
        if (success_count + failure_count) > 0:
            compilation_stats[task['log_name']] = {'success': success_count, 'failed': failure_count}
        if updated_files:
            update_summary[task['log_name']] = len(updated_files)
            
    return timing_summary, compilation_stats, update_summary


# --- 摘要打印函数 (保持不变) ---
def _print_time_summary(timing_summary: Dict):
    if not timing_summary: return
    print("\n\n" + "="*45 + "\n⏱️" + " "*14 + "编译时间摘要" + " "*15 + "⏱️\n" + "="*45)
    for name, (duration, count) in timing_summary.items():
        avg = f"平均: {(duration / count):.2f} 秒/文件" if count > 0 else ""
        print(f"- {name:<10} | 总耗时: {format_time(duration)} | {avg}")
    print("="*45)

def _print_stats_summary(stats: Dict):
    if not stats: return
    print("\n" + "="*45 + "\n📊" + " "*12 + "最终编译统计报告" + " "*13 + "📊\n" + "="*45)
    print(f"{'语言':<12} | {'✅ 成功':<10} | {'❌ 失败':<10}\n" + "-"*45)
    for lang, counts in stats.items():
        print(f"{lang:<12} | {counts.get('success', 0):<10} | {counts.get('failed', 0):<10}")
    print("="*45)

def _print_update_summary(update_summary: Dict):
    if not update_summary: return
    print("\n" + "="*45 + "\n🔄" + " "*14 + "更新文件统计" + " "*15 + "🔄\n" + "="*45)
    print(f"{'语言':<12} | {'更新数量':<10}\n" + "-"*45)
    for lang, count in update_summary.items():
        print(f"{lang:<12} | {count:<10}")
    print("="*45)