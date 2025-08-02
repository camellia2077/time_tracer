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
) -> Tuple[List[Tuple[Any, ...]], int]:
    """
    根据增量编译规则筛选任务。
    职责：只负责检查文件时间戳，过滤掉不需要重新编译的任务。

    返回:
        一个元组，包含 (需要运行的任务列表, 被跳过的文件数量)。
    """
    if not quiet: print("🔍 增量编译已启用，正在检查已存在的文件...")

    # 1. 高效预扫描：一次性获取所有输出文件的元数据
    output_file_metadata: Dict[str, float] = {}
    if os.path.exists(output_root_dir):
        for out_root, _, out_files in os.walk(output_root_dir):
            for out_file in out_files:
                if out_file.endswith('.pdf'):
                    pdf_path = os.path.join(out_root, out_file)
                    try:
                        output_file_metadata[pdf_path] = os.path.getmtime(pdf_path)
                    except FileNotFoundError:
                        continue # 在扫描和获取时间之间文件可能被删除

    # 2. 筛选任务
    final_tasks: List[Tuple[Any, ...]] = []
    skipped_count = 0
    for task in tasks:
        source_path, final_pdf_path = task[0], task[1]
        
        # 使用哈希表进行 O(1) 复杂度的快速查找
        if final_pdf_path in output_file_metadata:
            try:
                source_mtime = os.path.getmtime(source_path)
                output_mtime = output_file_metadata[final_pdf_path]
                
                # 如果源文件没有更新，则跳过
                if source_mtime < output_mtime:
                    skipped_count += 1
                    continue
            except FileNotFoundError:
                # 如果文件在此期间消失，则继续执行编译
                pass
        
        final_tasks.append(task)
        
    if not quiet and skipped_count > 0:
        print(f"✅ 已跳过 {skipped_count} 个未更改的文件。")
    
    return final_tasks, skipped_count