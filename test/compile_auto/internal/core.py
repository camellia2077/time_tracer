# internal/core.py
import os
import subprocess
import concurrent.futures
import time
from typing import Callable, List, Optional, Dict
from collections import defaultdict
from tqdm import tqdm # type: ignore

# compile_single_file 函数保持不变
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
            error_log = (f"\n{'='*20} 错误日志: {file_name} {'='*20}\n" f"❌ 失败: '{file_name}' 编译失败。返回码: {result.returncode} (耗时: {file_duration:.2f}秒)\n" f"--- {log_file_type} 编译器错误日志 ---\n" f"{result.stdout or '没有标准输出。'}\n" f"{result.stderr or '没有标准错误输出。'}\n" f"{'='*50}")
            return {"success": False, "file": file_name, "duration": file_duration, "log": error_log}
    except FileNotFoundError:
        return {"success": False, "file": file_name, "duration": 0, "log": f"❌ 错误：命令 '{command[0]}' 未找到。"}
    except Exception as e:
        return {"success": False, "file": file_name, "duration": 0, "log": f"❌ 处理文件 '{file_name}' 时发生未知错误: {e}"}

def _select_files_fairly(all_files: List[str], count: int) -> List[str]:
    """
    一个公平的文件选择器，它会从所有子目录中轮流挑选文件。
    """
    if not all_files:
        return []

    # 1. 按文件的直接父目录进行分组
    files_by_parent_dir: Dict[str, List[str]] = defaultdict(list)
    for f in all_files:
        parent_dir = os.path.dirname(f)
        files_by_parent_dir[parent_dir].append(f)

    # 2. 轮流挑选
    selected_files = []
    sorted_parent_dirs = sorted(files_by_parent_dir.keys())
    dir_iterators = {d: iter(files_by_parent_dir[d]) for d in sorted_parent_dirs}

    while len(selected_files) < count:
        files_added_this_round = 0
        for parent_dir in sorted_parent_dirs:
            try:
                file = next(dir_iterators[parent_dir])
                selected_files.append(file)
                files_added_this_round += 1
                if len(selected_files) >= count:
                    break
            except StopIteration:
                continue
        
        if files_added_this_round == 0:
            break
            
    return selected_files


def process_directory(
    source_dir: str,
    base_output_dir: str,
    file_extension: str,
    log_file_type: str,
    command_builder: Callable[[str, str, str], List[str]],
    max_workers: Optional[int] = None,
    post_process_hook: Optional[Callable[[str], None]] = None,
    enable_partial_compile: bool = False,
    partial_compile_count: int = 1
) -> int:
    """通用的文件编译处理函数，支持部分编译模式。"""
    source_dir = os.path.abspath(source_dir)
    source_folder_name = os.path.basename(source_dir)
    type_specific_output_root = os.path.join(base_output_dir, source_folder_name)

    worker_count = max_workers or os.cpu_count()
    print(f"\n===== 开始处理 {log_file_type} 文件 (使用最多 {worker_count} 个并行任务) =====")
    print(f"源目录: '{source_dir}'")
    print(f"输出到: '{type_specific_output_root}'")

    # --- 核心逻辑修改：为每个顶层分组独立调用公平选择器 ---

    # 1. 按顶层目录对所有文件进行分组
    files_by_top_level_dir: Dict[str, List[str]] = defaultdict(list)
    for root, dirs, files in os.walk(source_dir):
        dirs.sort()
        files.sort()
        relative_path = os.path.relpath(root, source_dir)
        # 确定分组的key (顶层目录名，或'.'代表根目录下的文件)
        top_level_key = relative_path.split(os.path.sep)[0] if os.path.sep in relative_path and relative_path != '.' else '.'
        for file in files:
            if file.endswith(file_extension):
                files_by_top_level_dir[top_level_key].append(os.path.join(root, file))
    
    # 将根目录下的文件单独作为分组
    if '.' in files_by_top_level_dir:
        root_files = files_by_top_level_dir.pop('.')
        for f in root_files:
            files_by_top_level_dir[os.path.basename(f)] = [f]

    files_to_process = []
    # 2. 根据模式选择处理方式
    if enable_partial_compile:
        count = max(1, partial_compile_count)
        print(f"⚠️  部分编译模式已开启，将为每个顶级子目录最多编译 {count} 个文件。")
        
        for group_name in sorted(files_by_top_level_dir.keys()):
            all_files_in_group = files_by_top_level_dir[group_name]
            # 为每个分组独立调用公平选择器
            selected = _select_files_fairly(all_files_in_group, count)
            files_to_process.extend(selected)
            print(f"   - 从分组 '{group_name}' 中选择了 {len(selected)} 个文件。")

    else:
        print("部分编译模式已关闭，将编译所有找到的文件。")
        for group in sorted(files_by_top_level_dir.keys()):
            files_to_process.extend(files_by_top_level_dir[group])


    if not files_to_process:
        print(f"\n在目录 '{source_dir}' 中没有找到任何 {file_extension} 文件。")
        return 0

    # --- 后续的创建任务和并行执行逻辑保持不变 ---
    tasks = []
    for input_path in files_to_process:
        relative_path_to_root = os.path.relpath(os.path.dirname(input_path), source_dir)
        target_output_dir = os.path.join(type_specific_output_root, relative_path_to_root)
        pdf_filename = os.path.splitext(os.path.basename(input_path))[0] + '.pdf'
        final_pdf_path = os.path.join(target_output_dir, pdf_filename)
        tasks.append((input_path, final_pdf_path, target_output_dir))

    success_count = 0
    with concurrent.futures.ProcessPoolExecutor(max_workers=max_workers) as executor:
        future_to_file = {
            executor.submit(compile_single_file, input_p, output_p, target_d, command_builder, log_file_type): os.path.basename(input_p)
            for input_p, output_p, target_d in tasks
        }
        
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
                    progress_bar.set_postfix_str(f"{result['log']} ({result['duration']:.2f}s)")
                else:
                    tqdm.write(result["log"])

            except Exception as e:
                file_name = future_to_file[future]
                tqdm.write(f"❌ 处理 '{file_name}' 时发生严重错误: {e}")

    if post_process_hook:
        post_process_hook(type_specific_output_root)

    return success_count