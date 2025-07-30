# internal/handlers.py
import os
import time
import argparse
from typing import Callable, List, Tuple, Dict, Any

from .core import process_directory
from .compilers import build_tex_command, build_typ_command, PandocCommandBuilder

def handle_tex(args):
    start_time = time.perf_counter()
    def cleanup_temp_files(directory: str):
        extensions = ['.aux', '.log', '.out']
        print(f"\n--- 在 '{directory}' 中清理临时文件 ---")
        deleted_count = 0
        for root, _, files in os.walk(directory):
            for file in files:
                if any(file.endswith(ext) for ext in extensions):
                    path = os.path.join(root, file)
                    try:
                        os.remove(path)
                        deleted_count += 1
                    except OSError as e:
                        print(f"❌ 错误：无法删除文件 '{path}': {e}")
        if deleted_count > 0: print(f"🗑️ 共清理了 {deleted_count} 个临时文件。")
        else: print("没有找到需要清理的临时文件。")
        print("--- 清理完成 ---")
        
    file_count = process_directory(
        source_dir=args.source_dir,
        base_output_dir=args.output_dir,
        file_extension='.tex',
        log_file_type='TeX',
        command_builder=build_tex_command,
        max_workers=args.jobs,
        post_process_hook=cleanup_temp_files,
        # --- 已修正：添加缺失的参数传递 ---
        enable_partial_compile=args.enable_partial_compile,
        partial_compile_count=args.partial_compile_count
    )
    end_time = time.perf_counter()
    if file_count > 0:
        print(f"===== TeX 文件处理完成 (共 {file_count} 个文件，总耗时: {end_time - start_time:.2f}秒) =====")
    return file_count

def handle_md(args):
    start_time = time.perf_counter()
    print(f"将使用字体: '{args.font}'")
    command_builder_instance = PandocCommandBuilder(source_format='gfm', font=args.font)
    file_count = process_directory(
        source_dir=args.source_dir,
        base_output_dir=args.output_dir,
        file_extension='.md',
        log_file_type='Markdown',
        command_builder=command_builder_instance,
        max_workers=args.jobs,
        enable_partial_compile=args.enable_partial_compile,
        partial_compile_count=args.partial_compile_count
    )
    end_time = time.perf_counter()
    if file_count > 0:
        print(f"===== Markdown 文件处理完成 (共 {file_count} 个文件，总耗时: {end_time - start_time:.2f}秒) =====")
    return file_count

def handle_rst(args):
    start_time = time.perf_counter()
    print(f"将使用字体: '{args.font}'")
    command_builder_instance = PandocCommandBuilder(source_format='rst', font=args.font)
    file_count = process_directory(
        source_dir=args.source_dir,
        base_output_dir=args.output_dir,
        file_extension='.rst',
        log_file_type='RST',
        command_builder=command_builder_instance,
        max_workers=args.jobs,
        enable_partial_compile=args.enable_partial_compile,
        partial_compile_count=args.partial_compile_count
    )
    end_time = time.perf_counter()
    if file_count > 0:
        print(f"===== RST 文件处理完成 (共 {file_count} 个文件，总耗时: {end_time - start_time:.2f}秒) =====")
    return file_count

def handle_typ(args):
    start_time = time.perf_counter()
    file_count = process_directory(
        source_dir=args.source_dir,
        base_output_dir=args.output_dir,
        file_extension='.typ',
        log_file_type='Typst',
        command_builder=build_typ_command,
        max_workers=args.jobs,
        enable_partial_compile=args.enable_partial_compile,
        partial_compile_count=args.partial_compile_count
    )
    end_time = time.perf_counter()
    if file_count > 0:
        print(f"===== Typst 文件处理完成 (共 {file_count} 个文件，总耗时: {end_time - start_time:.2f}秒) =====")
    return file_count

def _discover_tasks(source_dir: str, compiler_map: Dict, types_to_compile: List[str]) -> List[Dict[str, Any]]:
    tasks = []
    types_to_process_lower = [t.lower() for t in types_to_compile]
    print(f"注意：根据配置，将只编译以下类型 -> {types_to_compile}")
    for subdir_name in os.listdir(source_dir):
        full_subdir_path = os.path.join(source_dir, subdir_name)
        if not os.path.isdir(full_subdir_path):
            continue
        base_name_to_match = subdir_name.split('_')[0].lower()
        for keywords, (log_name, handler_func) in compiler_map.items():
            if base_name_to_match in keywords:
                if log_name.lower() not in types_to_process_lower:
                    print(f"\n>>> 检测到 '{subdir_name}' -> 类型 '{log_name}' 不在编译列表中，已跳过。")
                else:
                    print(f"\n>>> 检测到 '{subdir_name}' -> 将使用 {log_name} 编译器...")
                    tasks.append({
                        'log_name': log_name,
                        'handler_func': handler_func,
                        'source_path': full_subdir_path
                    })
                break
    return tasks

def _execute_tasks(tasks: List[Dict[str, Any]], args: argparse.Namespace) -> Dict:
    timing_summary = {}
    for task in tasks:
        print(f"\n--- 开始编译 {task['log_name']} ---")
        mock_args = argparse.Namespace(
            source_dir=task['source_path'],
            font=args.font,
            output_dir=args.output_dir,
            jobs=args.jobs,
            enable_partial_compile=args.enable_partial_compile,
            partial_compile_count=args.partial_compile_count
        )
        format_start_time = time.perf_counter()
        file_count = task['handler_func'](mock_args)
        format_end_time = time.perf_counter()
        if file_count > 0:
            timing_summary[task['log_name']] = (format_end_time - format_start_time, file_count)
    return timing_summary

def _print_summary(timing_summary: Dict):
    if not timing_summary:
        return
    print("\n\n" + "="*35)
    print("     自动模式编译时间摘要")
    print("="*35)
    for format_name, (duration, count) in timing_summary.items():
        avg_time_str = f"平均: {(duration / count):.2f} 秒/文件" if count > 0 else "无文件编译"
        print(f"- {format_name:<10} | 总耗时: {duration:>7.2f} 秒 | {avg_time_str}")
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
        print(f"\n在 '{parent_dir}' 中没有找到任何需要编译的目录。")
        print(f"(当前配置编译类型: {args.compile_types})")
        return
    summary = _execute_tasks(tasks_to_run, args)
    _print_summary(summary)