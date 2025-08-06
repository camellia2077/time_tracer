# internal/task_utils.py (最终重构版)
import os
from typing import List, Tuple, Dict, Any

class TaskProcessor:
    """
    一个用于发现和筛选文件编译任务的处理器。

    这个类封装了与特定源目录和输出目录相关的所有任务发现和增量检查逻辑。
    通过在初始化时设置好所有路径，后续的方法调用将变得更加简洁和专注。
    """

    def __init__(self, source_dir: str, base_output_dir: str, file_extension: str):
        """
        初始化任务处理器。

        Args:
            source_dir: 源文件所在的目录。
            base_output_dir: 所有输出的根目录。
            file_extension: 要查找的源文件的扩展名 (例如, '.md')。
        """
        if not os.path.isdir(source_dir):
            raise ValueError(f"提供的源目录不存在或不是一个有效目录: {source_dir}")

        self.source_dir = source_dir
        self.base_output_dir = base_output_dir
        self.file_extension = file_extension
        
        # 预先计算并存储本次处理的特定输出根目录
        source_folder_name = os.path.basename(source_dir)
        self.type_specific_output_root = os.path.join(base_output_dir, source_folder_name)

    def discover(self) -> List[Tuple[str, str, str]]:
        """
        发现所有需要编译的潜在任务。
        职责：遍历源目录，生成任务列表。
        """
        tasks: List[Tuple[str, str, str]] = []
        for root, _, files in os.walk(self.source_dir):
            for file in files:
                if file.endswith(self.file_extension):
                    input_path = os.path.join(root, file)
                    relative_path_dir = os.path.relpath(root, self.source_dir)
                    target_output_dir = os.path.join(self.type_specific_output_root, relative_path_dir)
                    
                    output_filename = os.path.splitext(file)[0] + '.pdf'
                    final_pdf_path = os.path.join(target_output_dir, output_filename)
                    
                    tasks.append((input_path, final_pdf_path, target_output_dir))
        return tasks

    def filter_for_incremental_compile(
        self, 
        tasks: List[Tuple[Any, ...]], 
        quiet: bool = False
    ) -> Tuple[List[Tuple[Any, ...]], int, List[str]]:
        """
        根据增量编译规则筛选任务。
        职责：协调辅助函数，遍历任务列表并根据文件时间戳做出决策。
        """
        if not quiet:
            print(f"🔍 增量编译已启用，正在检查 '{os.path.basename(self.source_dir)}' 的文件状态...")

        output_metadata = self._get_output_file_metadata()

        final_tasks: List[Tuple[Any, ...]] = []
        updated_source_paths: List[str] = []
        skipped_count = 0

        for task in tasks:
            source_path, final_pdf_path = task[0], task[1]
            
            if final_pdf_path not in output_metadata:
                final_tasks.append(task)
                continue

            try:
                if self._is_source_older_than_output(source_path, output_metadata[final_pdf_path]):
                    skipped_count += 1
                else:
                    if not quiet:
                        print(f"🔄 源文件 '{os.path.basename(source_path)}' 已更新，将重新编译...")
                    os.remove(final_pdf_path)
                    updated_source_paths.append(source_path)
                    final_tasks.append(task)
            
            except (FileNotFoundError, OSError) as e:
                print(f"⚠️ 警告：在检查 '{os.path.basename(source_path)}' 时发生错误: {e}。将执行编译。")
                final_tasks.append(task)
            
        if not quiet and skipped_count > 0:
            print(f"✅ 已跳过 {skipped_count} 个未更改的文件。")
        
        return final_tasks, skipped_count, updated_source_paths

    def _get_output_file_metadata(self) -> Dict[str, float]:
        """私有辅助函数：收集所有已存在输出PDF文件的时间戳。"""
        metadata: Dict[str, float] = {}
        if not os.path.exists(self.type_specific_output_root):
            return metadata

        for root, _, files in os.walk(self.type_specific_output_root):
            for file in files:
                if file.endswith('.pdf'):
                    pdf_path = os.path.join(root, file)
                    try:
                        metadata[pdf_path] = os.path.getmtime(pdf_path)
                    except FileNotFoundError:
                        continue
        return metadata
    
    @staticmethod
    def _is_source_older_than_output(source_path: str, output_mtime: float) -> bool:
        """私有静态方法：比较源文件和输出文件的时间戳。"""
        source_mtime = os.path.getmtime(source_path)
        return source_mtime < output_mtime