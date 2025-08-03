# internal/core.py (重构后)
import os
import subprocess
import concurrent.futures
import time
import tempfile
from typing import Callable, List, Optional, Tuple, Any, Dict
from tqdm import tqdm # type: ignore

# (修改后) 导入新的 compilers 函数
from .compilers import get_typst_template_content, build_md_to_typ_command, build_typ_command
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
    (修改后) 通过 'md -> typ -> pdf' 流程编译单个文件。
    此版本使用 with 语句来安全地管理一个临时的模板文件，
    修复了在 Windows 上的文件权限问题，并确保自动清理。
    """
    file_name = os.path.basename(input_path)
    typ_filename = os.path.splitext(file_name)[0] + '.typ'
    intermediate_typ_path = os.path.join(target_output_dir, typ_filename)

    try:
        os.makedirs(target_output_dir, exist_ok=True)
    except OSError as e:
        return {"success": False, "file": file_name, "log": f"❌ 错误：无法创建输出子目录 '{target_output_dir}': {e}"}

    # --- 步骤 1: MD -> Typst ---
    conversion_duration = 0.0
    temp_template_path = None # 初始化临时文件路径

    try:
        # 1. 创建临时文件，设置 delete=False 以便手动控制其生命周期
        with tempfile.NamedTemporaryFile(mode='w', suffix='.typ', encoding='utf-8', delete=False) as temp_template:
            temp_template_path = temp_template.name
            template_content = get_typst_template_content(font)
            temp_template.write(template_content)
            temp_template.close() # 2. 关键：关闭文件句柄以释放锁

        # 3. 现在 Pandoc 可以自由访问已关闭但存在的文件
        conversion_command = build_md_to_typ_command(input_path, intermediate_typ_path, temp_template_path)
        
        conv_start_time = time.perf_counter()
        conv_result = subprocess.run(conversion_command, capture_output=True, text=True, encoding='utf-8')
        conversion_duration = time.perf_counter() - conv_start_time
        
        if conv_result.returncode != 0:
            return {"success": False, "file": file_name, "conversion_time": conversion_duration, "log": f"❌ 步骤 1/2 (MD->Typ) 失败: {conv_result.stderr or conv_result.stdout}"}

    except (IOError, OSError) as e:
        return {"success": False, "file": file_name, "log": f"❌ 步骤 1/2 (MD->Typ) 创建临时模板失败: {e}"}
    finally:
        # 4. 关键：在 finally 块中确保临时模板文件总能被删除
        if temp_template_path and os.path.exists(temp_template_path):
            try:
                os.remove(temp_template_path)
            except OSError:
                pass

    # --- 步骤 2: Typst -> PDF ---
    compile_command = build_typ_command(intermediate_typ_path, final_pdf_path, None)
    comp_start_time = time.perf_counter()
    comp_result = subprocess.run(compile_command, capture_output=True, text=True, encoding='utf-8')
    compilation_duration = time.perf_counter() - comp_start_time
    
    # 清理中间生成的 .typ 文件 (这一步仍然需要)
    if os.path.exists(intermediate_typ_path):
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
    (修改后) 处理 'md -> typ -> pdf' 流程。
    此版本不再包含任何手动清理共享模板文件的逻辑。
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
    if not initial_tasks:
        if not quiet: print(f"\n在 '{source_dir}' 中没有找到 .md 文件。")
        return [], 0.0
        
    tasks_with_font = [task + (font,) for task in initial_tasks]
    
    tasks_to_run = tasks_with_font
    skipped_count = 0
    if incremental:
        tasks_to_run, skipped_count = filter_incremental_tasks(tasks_to_run, type_specific_output_root, quiet)

    results: List[Dict] = [{"success": True, "skipped": True}] * skipped_count
    
    if not tasks_to_run:
        if not quiet: print("\n所有文件都已是最新版本，无需编译。")
        return results, time.perf_counter() - dir_start_time

    # 【核心修改点】移除了整个 try...finally 结构，因为清理逻辑已下放到 compile_md_via_typ 中
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
    
    dir_duration = time.perf_counter() - dir_start_time
    return results, dir_duration