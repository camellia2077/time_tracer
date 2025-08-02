# internal/core.py (重构后)
import os
import subprocess
import concurrent.futures
import time
from typing import Callable, List, Optional, Tuple, Any, Dict
from tqdm import tqdm # type: ignore

from .compilers import build_md_to_typ_command, build_typ_command
# --- 新增: 导入新的工具函数 ---
from .task_utils import discover_tasks, filter_incremental_tasks

def compile_single_file(input_path: str, final_pdf_path: str, target_output_dir: str, command_builder: Callable, log_file_type: str) -> dict:
    """这个函数的职责很单一（编译单个文件），保持不变。"""
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
    【修改后】这个函数现在只负责编译，不再管理共享模板文件的清理。
    """
    file_name = os.path.basename(input_path)
    typ_filename = os.path.splitext(file_name)[0] + '.typ'
    intermediate_typ_path = os.path.join(target_output_dir, typ_filename)

    try:
        os.makedirs(target_output_dir, exist_ok=True)
    except OSError as e:
        return {"success": False, "file": file_name, "log": f"❌ 错误：无法创建输出子目录 '{target_output_dir}': {e}"}
    
    conversion_command = build_md_to_typ_command(input_path, intermediate_typ_path, None, font=font)
    conv_start_time = time.perf_counter()
    conv_result = subprocess.run(conversion_command, capture_output=True, text=True, encoding='utf-8')
    conversion_duration = time.perf_counter() - conv_start_time
    if conv_result.returncode != 0:
        return {"success": False, "file": file_name, "conversion_time": conversion_duration, "log": f"❌ 步骤 1/2 (MD->Typ) 失败: {conv_result.stderr or conv_result.stdout}"}

    compile_command = build_typ_command(intermediate_typ_path, final_pdf_path, None)
    comp_start_time = time.perf_counter()
    comp_result = subprocess.run(compile_command, capture_output=True, text=True, encoding='utf-8')
    compilation_duration = time.perf_counter() - comp_start_time
    
    # --- 修改点 ---
    # 只清理属于自己的中间文件，不再删除共享的模板文件 typst_template.typ
    try: 
        os.remove(intermediate_typ_path)
    except OSError: 
        pass

    if comp_result.returncode != 0:
        return {"success": False, "file": file_name, "conversion_time": conversion_duration, "compilation_time": compilation_duration, "log": f"❌ 步骤 2/2 (Typ->PDF) 失败: {comp_result.stderr or comp_result.stdout}"}
    
    return {"success": True, "file": file_name, "conversion_time": conversion_duration, "compilation_time": compilation_duration, "total_time": conversion_duration + compilation_duration, "log": f"✅ 成功: '{file_name}'"}

def process_directory(
    source_dir: str, base_output_dir: str, file_extension: str, log_file_type: str,
    command_builder: Callable[[str, str, str], List[str]], max_workers: Optional[int] = None,
    post_process_hook: Optional[Callable[[str], None]] = None, quiet: bool = False,
    incremental: bool = True
) -> Tuple[int, float]:
    """
    【重构后】处理一个目录的编译任务，现在只负责协调。 (此函数无需修改)
    """
    dir_start_time = time.perf_counter()
    worker_count = max_workers or os.cpu_count()
    source_folder_name = os.path.basename(os.path.abspath(source_dir))
    type_specific_output_root = os.path.join(base_output_dir, source_folder_name)

    if not quiet:
        print(f"\n===== 开始处理 {log_file_type} (最多 {worker_count} 个并行任务) =====")
        print(f"源: '{os.path.abspath(source_dir)}' -> 输出: '{type_specific_output_root}'")

    # 1. 发现任务 (职责已分离)
    tasks_to_run = discover_tasks(source_dir, base_output_dir, file_extension)
    if not tasks_to_run:
        if not quiet: print(f"\n在 '{source_dir}' 中没有找到 {file_extension} 文件。")
        return 0, 0.0

    # 2. 增量检查 (职责已分离)
    if incremental:
        tasks_to_run, _ = filter_incremental_tasks(tasks_to_run, type_specific_output_root, quiet)

    if not tasks_to_run:
        if not quiet: print("\n所有文件都已是最新版本，无需编译。")
        return 0, 0.0

    # 3. 执行编译 (核心职责)
    success_count = 0
    with concurrent.futures.ProcessPoolExecutor(max_workers=max_workers) as executor:
        future_to_file = {executor.submit(compile_single_file, *task, command_builder, log_file_type): task[0] for task in tasks_to_run}
        progress_bar = tqdm(concurrent.futures.as_completed(future_to_file), total=len(tasks_to_run), desc=f"编译 {log_file_type}", unit="file", disable=quiet)
        for future in progress_bar:
            try:
                result = future.result()
                if result["success"]:
                    success_count += 1
                    if not quiet: progress_bar.set_postfix_str(f"{result['log']} ({result['duration']:.2f}s)")
                else: tqdm.write(result["log"])
            except Exception as e:
                tqdm.write(f"❌ 处理时发生严重错误: {e}")

    # 4. 后处理
    if post_process_hook: post_process_hook(type_specific_output_root)
    dir_duration = time.perf_counter() - dir_start_time
    return success_count, dir_duration

def process_directory_md_via_typ(
    source_dir: str, base_output_dir: str, font: str, max_workers: Optional[int] = None, quiet: bool = False,
    incremental: bool = True
) -> Tuple[List[dict], float]:
    """
    【修改后】处理 'md -> typ -> pdf' 流程，负责管理共享模板的生命周期。
    """
    dir_start_time = time.perf_counter()
    worker_count = max_workers or os.cpu_count()
    source_folder_name = os.path.basename(os.path.abspath(source_dir))
    type_specific_output_root = os.path.join(base_output_dir, source_folder_name)
    
    if not quiet:
        print(f"\n===== 开始处理 MD->Typ->PDF (最多 {worker_count} 个并行任务) =====")
        print(f"源: '{os.path.abspath(source_dir)}' -> 输出: '{type_specific_output_root}'")

    try:
        os.makedirs(type_specific_output_root, exist_ok=True)
    except OSError as e:
        if not quiet: print(f"❌ 致命错误: 无法创建根输出目录 '{type_specific_output_root}': {e}")
        return [], time.perf_counter() - dir_start_time

    initial_tasks = discover_tasks(source_dir, base_output_dir, '.md')
    tasks_with_font = [task + (font,) for task in initial_tasks]
    
    if not tasks_with_font:
        if not quiet: print(f"\n在 '{source_dir}' 中没有找到 .md 文件。")
        return [], 0.0

    tasks_to_run = tasks_with_font
    skipped_count = 0
    if incremental:
        tasks_to_run, skipped_count = filter_incremental_tasks(tasks_to_run, type_specific_output_root, quiet)

    results: List[Dict] = [{"success": True, "skipped": True}] * skipped_count
    
    if not tasks_to_run:
        if not quiet: print("\n所有文件都已是最新版本，无需编译。")
        # 注意: 如果没有任务运行，也就不存在模板文件，直接返回是安全的。
        return results, time.perf_counter() - dir_start_time

    # --- 核心修改点: 更新 finally 块中的清理逻辑 ---
    try:
        with concurrent.futures.ProcessPoolExecutor(max_workers=max_workers) as executor:
            future_to_file = {executor.submit(compile_md_via_typ, *task): task[0] for task in tasks_to_run}
            progress_bar = tqdm(concurrent.futures.as_completed(future_to_file), total=len(tasks_to_run), desc="编译 MD->Typ->PDF", unit="file", disable=quiet)
            for future in progress_bar:
                try:
                    result = future.result()
                    results.append(result)
                    if result.get("success") and not result.get("skipped") and not quiet:
                        progress_bar.set_postfix_str(f"{result['log']} (总耗时: {result.get('total_time', 0):.2f}s)")
                    elif not result.get("success"):
                        tqdm.write(result["log"])
                except Exception as e:
                    tqdm.write(f"❌ 处理时发生严重错误: {e}")
    finally:
        # 【新逻辑】任务结束后，递归扫描并清理所有可能被创建的模板文件。
        # 这个方法对于扁平或嵌套目录结构都同样有效。
        template_filename = "typst_template.typ"
        if os.path.exists(type_specific_output_root):
            if not quiet:
                # 仅为调试和确认，可以取消下面的注释
                # tqdm.write(f"--- 正在清理临时文件 '{template_filename}' ---")
                pass
            for root, _, files in os.walk(type_specific_output_root):
                if template_filename in files:
                    file_to_delete = os.path.join(root, template_filename)
                    try:
                        os.remove(file_to_delete)
                    except OSError:
                        if not quiet:
                            tqdm.write(f"警告: 无法删除临时模板文件 '{file_to_delete}'。")
    
    dir_duration = time.perf_counter() - dir_start_time
    return results, dir_duration