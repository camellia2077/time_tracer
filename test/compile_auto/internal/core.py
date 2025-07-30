# internal/core.py
import os
import subprocess
import concurrent.futures
import time  # time is used in compile_single_file but was missing from the import list
from typing import Callable, List, Optional
from tqdm import tqdm # type: ignore # <--- 1. 导入 tqdm

def compile_single_file(input_path: str, final_pdf_path: str, target_output_dir: str, command_builder: Callable, log_file_type: str) -> dict:
    """
    编译单个文件并返回结果。此函数将在子进程中运行。
    """
    file_name = os.path.basename(input_path)
    command = command_builder(input_path, final_pdf_path, target_output_dir)
    
    try:
        os.makedirs(target_output_dir, exist_ok=True)
    except OSError as e:
        return {
            "success": False,
            "file": file_name,
            "duration": 0,
            "log": f"❌ 错误：无法创建输出子目录 '{target_output_dir}': {e}"
        }

    try:
        file_start_time = time.perf_counter()
        result = subprocess.run(
            command, capture_output=True, text=True, encoding='utf-8'
        )
        file_duration = time.perf_counter() - file_start_time

        if result.returncode == 0:
            # 对于成功的情况，我们可以只返回文件名和耗时，让主循环格式化
            return {
                "success": True,
                "file": file_name,
                "duration": file_duration,
                "log": f"✅ 成功: '{file_name}'"
            }
        else:
            # 失败时，日志更重要
            error_log = (
                f"\n{'='*20} 错误日志: {file_name} {'='*20}\n"
                f"❌ 失败: '{file_name}' 编译失败。返回码: {result.returncode} (耗时: {file_duration:.2f}秒)\n"
                f"--- {log_file_type} 编译器错误日志 ---\n"
                f"{result.stdout or '没有标准输出。'}\n"
                f"{result.stderr or '没有标准错误输出。'}\n"
                f"{'='*50}"
            )
            return {"success": False, "file": file_name, "duration": file_duration, "log": error_log}

    except FileNotFoundError:
        return {"success": False, "file": file_name, "duration": 0, "log": f"❌ 错误：命令 '{command[0]}' 未找到。"}
    except Exception as e:
        return {"success": False, "file": file_name, "duration": 0, "log": f"❌ 处理文件 '{file_name}' 时发生未知错误: {e}"}


def process_directory(
    source_dir: str,
    base_output_dir: str,
    file_extension: str,
    log_file_type: str,
    command_builder: Callable[[str, str, str], List[str]],
    max_workers: Optional[int] = None,
    post_process_hook: Optional[Callable[[str], None]] = None
) -> int:
    """通用的文件编译处理函数，使用tqdm显示并行处理进度，并返回成功处理的文件数量。"""
    source_dir = os.path.abspath(source_dir)
    source_folder_name = os.path.basename(source_dir)
    type_specific_output_root = os.path.join(base_output_dir, source_folder_name)

    worker_count = max_workers or os.cpu_count()
    print(f"\n===== 开始处理 {log_file_type} 文件 (使用最多 {worker_count} 个并行任务) =====")
    print(f"源目录: '{source_dir}'")
    print(f"输出到: '{type_specific_output_root}'")

    tasks = []
    for root, _, files in os.walk(source_dir):
        for file in files:
            if file.endswith(file_extension):
                input_path = os.path.join(root, file)
                relative_path = os.path.relpath(root, source_dir)
                target_output_dir = os.path.join(type_specific_output_root, relative_path)
                pdf_filename = os.path.splitext(file)[0] + '.pdf'
                final_pdf_path = os.path.join(target_output_dir, pdf_filename)
                tasks.append((input_path, final_pdf_path, target_output_dir))

    if not tasks:
        print(f"\n在目录 '{source_dir}' 中没有找到任何 {file_extension} 文件。")
        return 0

    success_count = 0
    with concurrent.futures.ProcessPoolExecutor(max_workers=max_workers) as executor:
        future_to_file = {
            executor.submit(compile_single_file, input_p, output_p, target_d, command_builder, log_file_type): os.path.basename(input_p)
            for input_p, output_p, target_d in tasks
        }
        
        # 2. 使用tqdm包装as_completed来创建进度条
        progress_bar = tqdm(
            concurrent.futures.as_completed(future_to_file), 
            total=len(tasks), 
            desc=f"编译 {log_file_type}",
            unit="file"
        )

        for future in progress_bar:
            try:
                result = future.result()
                if result["success"]:
                    success_count += 1
                    # 可以在进度条后面显示当前成功处理的文件名和耗时
                    progress_bar.set_postfix_str(f"{result['log']} ({result['duration']:.2f}s)")
                else:
                    # 3. 使用tqdm.write打印错误日志，避免破坏进度条
                    tqdm.write(result["log"])

            except Exception as e:
                file_name = future_to_file[future]
                tqdm.write(f"❌ 处理 '{file_name}' 时发生严重错误: {e}")

    if post_process_hook:
        post_process_hook(type_specific_output_root)

    return success_count