# internal/handlers.py
import os
import time
import argparse
import shutil
from typing import Callable, List, Tuple, Dict, Any

from .core import process_directory, process_directory_md_via_typ
from .compilers import build_tex_command, build_typ_command, PandocCommandBuilder

def format_time(seconds):
    """将秒数格式化为 HH:MM:SS """
    seconds = int(seconds)
    hours = seconds // 3600
    minutes = (seconds % 3600) // 60
    seconds = seconds % 60
    return f"{hours:02d}:{minutes:02d}:{seconds:02d}"

def handle_tex(args):
    """
    处理 TeX 文件编译，并在完成后清理临时文件。
    """
    def cleanup_temp_files(directory: str):
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
                        print(f"🗑️ 已删除: {path}")
                    except OSError as e:
                        print(f"❌ 错误：无法删除文件 '{path}': {e}")
        
        if deleted_count == 0:
            print("没有找到需要清理的临时文件。")
        else:
            print(f"--- 清理完成，共删除 {deleted_count} 个文件 ---")

    file_count, duration = process_directory(
        source_dir=args.source_dir,
        base_output_dir=args.output_dir,
        file_extension='.tex',
        log_file_type='TeX',
        command_builder=build_tex_command,
        max_workers=args.jobs,
        post_process_hook=cleanup_temp_files,
        incremental=args.incremental # <--- 传递开关
    )
    
    if file_count > 0:
        print(f"===== TeX 处理完成 (共 {file_count} 个文件) =====")
        
    return file_count, duration

def handle_rst(args):
    print(f"将使用字体: '{args.font}'")
    builder = PandocCommandBuilder(source_format='rst', font=args.font)
    file_count, duration = process_directory(
        source_dir=args.source_dir, base_output_dir=args.output_dir, file_extension='.rst',
        log_file_type='RST', command_builder=builder, max_workers=args.jobs,
        incremental=args.incremental # <--- 传递开关
    )
    if file_count > 0: print(f"===== RST 处理完成 (共 {file_count} 个文件) =====")
    return file_count, duration

def handle_typ(args):
    file_count, duration = process_directory(
        source_dir=args.source_dir, base_output_dir=args.output_dir, file_extension='.typ',
        log_file_type='Typst', command_builder=build_typ_command, max_workers=args.jobs,
        incremental=args.incremental # <--- 传递开关
    )
    if file_count > 0: print(f"===== Typst 处理完成 (共 {file_count} 个文件) =====")
    return file_count, duration

def _run_benchmark(args: argparse.Namespace):
    """执行 Markdown 编译基准测试。"""
    print("\n" + "="*50)
    print(f"🚀  启动 Markdown 编译基准测试模式  🚀")
    print(f"   比较方法: {', '.join(args.markdown_compilers)}")
    print(f"   循环次数: {args.benchmark_loops} 次")
    print(f"   使用字体: '{args.font}'")
    print("="*50)

    benchmark_results: Dict[str, List[float]] = {compiler: [] for compiler in args.markdown_compilers}
    
    for i in range(args.benchmark_loops):
        print(f"\n--- 第 {i + 1}/{args.benchmark_loops} 轮测试 ---")
        for compiler in args.markdown_compilers:
            target_output_path = os.path.join(args.output_dir, os.path.basename(args.source_dir))
            if os.path.exists(target_output_path):
                shutil.rmtree(target_output_path)
            
            print(f"  > 正在测试: {compiler}...")
            duration = 0.0
            if compiler == 'pandoc':
                builder = PandocCommandBuilder(source_format='gfm', font=args.font)
                # 基准测试总是完全编译，所以 incremental=False
                _, duration = process_directory(
                    args.source_dir, args.output_dir, '.md', 'Markdown', builder, args.jobs, quiet=True, incremental=False
                )
            elif compiler == 'typst':
                # 基准测试总是完全编译，所以 incremental=False
                _, duration = process_directory_md_via_typ(
                    args.source_dir, args.output_dir, font=args.font, max_workers=args.jobs, quiet=True, incremental=False
                )
            
            benchmark_results[compiler].append(duration)
            print(f"    本轮耗时: {duration:.4f} 秒")

    _print_benchmark_summary(benchmark_results)
    return 1

def _print_benchmark_summary(results: Dict[str, List[float]]):
    print("\n" + "="*50)
    print("📊  基准测试结果摘要  📊")
    print("="*50)
    total_times = {compiler: sum(durations) for compiler, durations in results.items()}
    for compiler, total_time in total_times.items():
        avg_time = total_time / len(results[compiler])
        print(f"方法: {compiler.upper()}")
        print(f"  - 总耗时: {total_time:.4f} 秒")
        print(f"  - 平均耗时: {avg_time:.4f} 秒/轮")
    if len(total_times) > 1:
        best_compiler = min(total_times, key=total_times.get)
        print("-" * 50)
        print(f"🏆 结论: [{best_compiler.upper()}] 性能更优！")
    print("="*50)

def handle_md(args: argparse.Namespace):
    """Markdown 处理器，根据配置选择常规编译或基准测试。"""
    compilers = getattr(args, 'markdown_compilers', ['pandoc'])
    
    if len(compilers) > 1 and "markdown" in [t.lower() for t in args.compile_types]:
        file_count = _run_benchmark(args)
        return file_count, 0.0
    
    else:
        compiler = compilers[0] if compilers else 'pandoc'
        file_count = 0
        duration = 0.0
        print(f"===== 开始处理 Markdown (使用 {compiler} 方式) =====")
        print(f"将使用字体: '{args.font}'")

        if compiler == 'typst':
            results, duration = process_directory_md_via_typ(
                source_dir=args.source_dir, base_output_dir=args.output_dir, 
                font=args.font, max_workers=args.jobs,
                incremental=args.incremental # <--- 传递开关
            )
            file_count = len(results)
            if file_count > 0:
                success_count = sum(1 for r in results if r.get("success"))
                conv_time = sum(r.get("conversion_time", 0) for r in results)
                comp_time = sum(r.get("compilation_time", 0) for r in results)
                print("\n--- Markdown (Typst 路径) 详细统计 ---")
                print(f"成功: {success_count} / {file_count} 个文件")
                print(f"总转换耗时 (MD->Typ): {format_time(conv_time)}")
                print(f"总编译耗时 (Typ->PDF): {format_time(comp_time)}")

        else: 
            builder = PandocCommandBuilder(source_format='gfm', font=args.font)
            file_count, duration = process_directory(
                source_dir=args.source_dir, base_output_dir=args.output_dir,
                file_extension='.md', log_file_type='Markdown',
                command_builder=builder, max_workers=args.jobs,
                incremental=args.incremental # <--- 传递开关
            )

        if file_count > 0:
            print(f"===== Markdown ({compiler}) 处理完成 =====")
        return file_count, duration

def _discover_tasks(source_dir: str, compiler_map: Dict, types_to_compile: List[str]) -> List[Dict[str, Any]]:
    tasks = []
    types_to_process_lower = [t.lower() for t in types_to_compile]
    print(f"注意：根据配置，将只编译以下类型 -> {types_to_compile}")
    for subdir_name in os.listdir(source_dir):
        full_subdir_path = os.path.join(source_dir, subdir_name)
        if not os.path.isdir(full_subdir_path): continue
        base_name_to_match = subdir_name.split('_')[0].lower()
        for keywords, (log_name, handler_func) in compiler_map.items():
            if base_name_to_match in keywords:
                if log_name.lower() in types_to_process_lower:
                    print(f"\n>>> 检测到 '{subdir_name}' -> 将使用 {log_name} 编译器...")
                    tasks.append({'log_name': log_name, 'handler_func': handler_func, 'source_path': full_subdir_path})
                else:
                    print(f"\n>>> 检测到 '{subdir_name}' -> 类型 '{log_name}' 不在编译列表中，已跳过。")
                break
    return tasks

def _execute_tasks(tasks: List[Dict[str, Any]], args: argparse.Namespace) -> Dict:
    timing_summary = {}
    for task in tasks:
        task_args = argparse.Namespace(**vars(args))
        task_args.source_dir = task['source_path']
        file_count, duration = task['handler_func'](task_args)
        if file_count > 0 and duration > 0:
            timing_summary[task['log_name']] = (duration, file_count)
    return timing_summary

def _print_summary(timing_summary: Dict):
    if not timing_summary: return
    print("\n\n" + "="*35)
    print("     常规模式编译时间摘要")
    print("="*35)
    for format_name, (duration, count) in timing_summary.items():
        avg_time_str = f"平均: {(duration / count):.2f} 秒/文件" if count > 0 else ""
        print(f"- {format_name:<10} | 总耗时: {format_time(duration)} | {avg_time_str}")
    print("="*35)

def handle_auto(args: argparse.Namespace):
    parent_dir = args.source_dir
    print(f"===== 启动自动编译模式 =====")
    print(f"扫描父目录: '{parent_dir}'")
    compiler_map: dict[Tuple[str, ...], Tuple[str, Callable]] = {
        ('latex', 'tex'): ('TeX', handle_tex),
        ('markdown', 'md'): ('Markdown', handle_md),
        ('rst', 'rest'): ('RST', handle_rst),
        ('typst', 'typ'): ('Typst', handle_typ)
    }
    tasks_to_run = _discover_tasks(parent_dir, compiler_map, args.compile_types)
    if not tasks_to_run:
        print(f"\n在 '{parent_dir}' 中没有找到任何需要编译的目录。 (配置类型: {args.compile_types})")
        return
    summary = _execute_tasks(tasks_to_run, args)
    if summary:
        _print_summary(summary)