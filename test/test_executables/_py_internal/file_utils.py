# _py_internal/file_utils.py
import os
from pathlib import Path

# 计算占用空间
def get_folder_size(folder_path: Path) -> int:
    """Calculates the total size of a folder and all its subfolders."""
    total_size = 0
    for dirpath, _, filenames in os.walk(folder_path):
        for f in filenames:
            fp = os.path.join(dirpath, f)
            # skip if it is symbolic link
            if not os.path.islink(fp):
                total_size += os.path.getsize(fp)
    return total_size
def format_size(size_in_bytes: int) -> str:
    """Formats a size in bytes to a human-readable string (KB, MB)."""
    if size_in_bytes < 1024:
        return f"{size_in_bytes} Bytes"
    elif size_in_bytes < 1024 * 1024:
        return f"{size_in_bytes / 1024:.2f} KB"
    else:
        return f"{size_in_bytes / (1024 * 1024):.2f} MB"