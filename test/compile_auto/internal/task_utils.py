# internal/task_utils.py

# discover_tasks: 负责扫描源目录，找出所有需要处理的文件。

# filter_incremental_tasks: 负责实现增量编译的检查逻辑。

import os
from typing import List, Tuple, Dict, Any

def discover_tasks(source_dir: str, base_output_dir: str, file_extension: str) -> List[Tuple[str, str, str]]:
    """
    发现需要编译的任务。
    职责：只负责遍历源目录，生成一个包含所有潜在任务的列表。

    返回:
        一个任务元组的列表，每个元组包含 (源文件路径, 目标PDF路径, 目标输出目录)。
    """
    initial_tasks: List[Tuple[str, str, str]] = []
    source_folder_name = os.path.basename(source_dir)
    type_specific_output_root = os.path.join(base_output_dir, source_folder_name)

    for root, _, files in os.walk(source_dir):
        for file in files:
            if file.endswith(file_extension):
                input_path = os.path.join(root, file)
                relative_path_dir = os.path.relpath(root, source_dir)
                target_output_dir = os.path.join(type_specific_output_root, relative_path_dir)
                output_filename = os.path.splitext(file)[0] + '.pdf'
                final_pdf_path = os.path.join(target_output_dir, output_filename)
                initial_tasks.append((input_path, final_pdf_path, target_output_dir))
    
    return initial_tasks

def filter_incremental_tasks(
    tasks: List[Tuple[Any, ...]], 
    output_root_dir: str, 
    quiet: bool = False
) -> Tuple[List[Tuple[Any, ...]], int, List[str]]:
    """
    【修改后】根据增量编译规则筛选任务。
    职责：检查文件时间戳。如果输出文件已过时，则删除它并准备重新编译。

    返回:
        一个元组，包含 (需要运行的任务列表, 被跳过的文件数量, 被更新的源文件路径列表)。
    """
    if not quiet: print("🔍 增量编译已启用，正在检查文件状态...")

    output_file_metadata: Dict[str, float] = {}
    if os.path.exists(output_root_dir):
        for out_root, _, out_files in os.walk(output_root_dir):
            for out_file in out_files:
                if out_file.endswith('.pdf'):
                    pdf_path = os.path.join(out_root, out_file)
                    try:
                        output_file_metadata[pdf_path] = os.path.getmtime(pdf_path)
                    except FileNotFoundError:
                        continue

    final_tasks: List[Tuple[Any, ...]] = []
    updated_source_paths: List[str] = [] # 新增：用于存储被更新的源文件
    skipped_count = 0
    for task in tasks:
        source_path, final_pdf_path = task[0], task[1]
        
        if final_pdf_path in output_file_metadata:
            try:
                source_mtime = os.path.getmtime(source_path)
                output_mtime = output_file_metadata[final_pdf_path]
                
                if source_mtime < output_mtime:
                    skipped_count += 1
                    continue
                else:
                    if not quiet:
                        print(f"🔄 源文件 '{os.path.basename(source_path)}' 已更新，将重新编译...")
                    os.remove(final_pdf_path)
                    updated_source_paths.append(source_path) # 新增：记录更新
            except (FileNotFoundError, OSError):
                pass
        
        final_tasks.append(task)
        
    if not quiet and skipped_count > 0:
        print(f"✅ 已跳过 {skipped_count} 个未更改的文件。")
    
    return final_tasks, skipped_count, updated_source_paths