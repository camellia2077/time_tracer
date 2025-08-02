# internal/core.py
import os
import subprocess
import concurrent.futures
import time
from typing import Callable, List, Optional, Tuple, Any
from tqdm import tqdm # type: ignore

from .compilers import build_md_to_typ_command, build_typ_command

def compile_single_file(input_path: str, final_pdf_path: str, target_output_dir: str, command_builder: Callable, log_file_type: str) -> dict:
    file_name = os.path.basename(input_path)
    command = command_builder(input_path, final_pdf_path, target_output_dir)
    try:
        os.makedirs(target_output_dir, exist_ok=True)
    except OSError as e:
        return {"success": False, "file": file_name, "duration": 0, "log": f"❌ 错误：无法创建输出子目录 '{target_output_dir}': {e}"}
    try:
        file_start_time = time.perf_counter()
        result = subprocess.run(command, capture_output=True, text=True, encoding='utf-8')
        file_duration = time.perf_counter() - file_start_time
        if result.returncode == 0:
            return {"success": True, "file": file_name, "duration": file_duration, "log": f"✅ 成功: '{file_name}'"}
        else:
            error_log = (f"\n{'='*20} 错误日志: {file_name} {'='*20}\n"
                         f"❌ 失败: '{file_name}' (耗时: {file_duration:.2f}s)\n"
                         f"--- {log_file_type} 编译器错误日志 ---\n{result.stderr or result.stdout}\n{'='*50}")
            return {"success": False, "file": file_name, "duration": file_duration, "log": error_log}
    except Exception as e:
        return {"success": False, "file": file_name, "duration": 0, "log": f"❌ 处理文件 '{file_name}' 时发生未知错误: {e}"}

def compile_md_via_typ(input_path: str, final_pdf_path: str, target_output_dir: str, font: str) -> dict:
    """
    通过 'md -> typ -> pdf' 流程编译单个文件。
    (新增) font 参数用于传递字体名称。
    """
    file_name = os.path.basename(input_path)
    typ_filename = os.path.splitext(file_name)[0] + '.typ'
    intermediate_typ_path = os.path.join(target_output_dir, typ_filename)
    try:
        os.makedirs(target_output_dir, exist_ok=True)
    except OSError as e:
        return {"success": False, "file": file_name, "log": f"❌ 错误：无法创建输出子目录 '{target_output_dir}': {e}"}
    
    # --- 步骤 1: MD -> Typst (现在会传入字体) ---
    conversion_command = build_md_to_typ_command(input_path, intermediate_typ_path, None, font=font)
    conv_start_time = time.perf_counter()
    conv_result = subprocess.run(conversion_command, capture_output=True, text=True, encoding='utf-8')
    conversion_duration = time.perf_counter() - conv_start_time
    if conv_result.returncode != 0:
        return {"success": False, "file": file_name, "conversion_time": conversion_duration, "log": f"❌ 步骤 1/2 (MD->Typ) 失败: {conv_result.stderr or conv_result.stdout}"}

    # --- 步骤 2: Typst -> PDF ---
    compile_command = build_typ_command(intermediate_typ_path, final_pdf_path, None)
    comp_start_time = time.perf_counter()
    comp_result = subprocess.run(compile_command, capture_output=True, text=True, encoding='utf-8')
    compilation_duration = time.perf_counter() - comp_start_time
    try: os.remove(intermediate_typ_path)
    except OSError: pass
    if comp_result.returncode != 0:
        return {"success": False, "file": file_name, "conversion_time": conversion_duration, "compilation_time": compilation_duration, "log": f"❌ 步骤 2/2 (Typ->PDF) 失败: {comp_result.stderr or comp_result.stdout}"}
    
    return {"success": True, "file": file_name, "conversion_time": conversion_duration, "compilation_time": compilation_duration, "total_time": conversion_duration + compilation_duration, "log": f"✅ 成功: '{file_name}'"}

def process_directory(
    source_dir: str, base_output_dir: str, file_extension: str, log_file_type: str,
    command_builder: Callable[[str, str, str], List[str]], max_workers: Optional[int] = None,
    post_process_hook: Optional[Callable[[str], None]] = None, quiet: bool = False
) -> Tuple[int, float]:
    dir_start_time = time.perf_counter()
    source_dir = os.path.abspath(source_dir)
    source_folder_name = os.path.basename(source_dir)
    type_specific_output_root = os.path.join(base_output_dir, source_folder_name)
    worker_count = max_workers or os.cpu_count()
    if not quiet:
        print(f"\n===== 开始处理 {log_file_type} (最多 {worker_count} 个并行任务) =====")
        print(f"源: '{source_dir}' -> 输出: '{type_specific_output_root}'")
    tasks = []
    for root, _, files in os.walk(source_dir):
        for file in files:
            if file.endswith(file_extension):
                tasks.append((os.path.join(root, file), os.path.join(type_specific_output_root, os.path.relpath(root, source_dir), os.path.splitext(file)[0] + '.pdf'), os.path.join(type_specific_output_root, os.path.relpath(root, source_dir))))
    if not tasks:
        if not quiet: print(f"\n在 '{source_dir}' 中没有找到 {file_extension} 文件。")
        return 0, 0.0
    success_count = 0
    with concurrent.futures.ProcessPoolExecutor(max_workers=max_workers) as executor:
        future_to_file = {executor.submit(compile_single_file, *task, command_builder, log_file_type): task[0] for task in tasks}
        progress_bar = tqdm(concurrent.futures.as_completed(future_to_file), total=len(tasks), desc=f"编译 {log_file_type}", unit="file", disable=quiet)
        for future in progress_bar:
            try:
                result = future.result()
                if result["success"]:
                    success_count += 1
                    if not quiet: progress_bar.set_postfix_str(f"{result['log']} ({result['duration']:.2f}s)")
                else: tqdm.write(result["log"])
            except Exception as e:
                tqdm.write(f"❌ 处理时发生严重错误: {e}")
    if post_process_hook: post_process_hook(type_specific_output_root)
    dir_duration = time.perf_counter() - dir_start_time
    return success_count, dir_duration

def process_directory_md_via_typ(
    source_dir: str, base_output_dir: str, font: str, max_workers: Optional[int] = None, quiet: bool = False
) -> Tuple[List[dict], float]:
    """
    处理 'md -> typ -> pdf' 流程的专用函数。
    (新增) font 参数。
    """
    dir_start_time = time.perf_counter()
    source_dir = os.path.abspath(source_dir)
    source_folder_name = os.path.basename(source_dir)
    type_specific_output_root = os.path.join(base_output_dir, source_folder_name)

    worker_count = max_workers or os.cpu_count()
    if not quiet:
        print(f"\n===== 开始处理 MD->Typ->PDF (最多 {worker_count} 个并行任务) =====")
        print(f"源: '{source_dir}' -> 输出: '{type_specific_output_root}'")

    tasks = []
    for root, _, files in os.walk(source_dir):
        for file in files:
            if file.endswith('.md'):
                input_path = os.path.join(root, file)
                relative_path = os.path.relpath(root, source_dir)
                target_output_dir = os.path.join(type_specific_output_root, relative_path)
                pdf_filename = os.path.splitext(file)[0] + '.pdf'
                final_pdf_path = os.path.join(target_output_dir, pdf_filename)
                tasks.append((input_path, final_pdf_path, target_output_dir, font))

    if not tasks:
        if not quiet: print(f"\n在 '{source_dir}' 中没有找到 .md 文件。")
        return [], 0.0

    results = []
    with concurrent.futures.ProcessPoolExecutor(max_workers=max_workers) as executor:
        future_to_file = {executor.submit(compile_md_via_typ, *task): task[0] for task in tasks}
        progress_bar = tqdm(concurrent.futures.as_completed(future_to_file), total=len(tasks), desc="编译 MD->Typ->PDF", unit="file", disable=quiet)
        for future in progress_bar:
            try:
                result = future.result()
                results.append(result)
                if result["success"] and not quiet:
                    progress_bar.set_postfix_str(f"{result['log']} (总耗时: {result['total_time']:.2f}s)")
                elif not result["success"]:
                    tqdm.write(result["log"])
            except Exception as e:
                tqdm.write(f"❌ 处理时发生严重错误: {e}")
    
    dir_duration = time.perf_counter() - dir_start_time
    return results, dir_duration