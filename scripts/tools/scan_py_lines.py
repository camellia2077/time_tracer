import os


def analyze_large_python_files(target_dir, min_lines=200):
    """
    扫描 Python 项目，查找行数超过阈值的文件
    """
    # 1. 过滤机制：Python 特有的无关目录
    ignore_dirs = {
        ".git",
        ".idea",
        ".vscode",
        "__pycache__",
        "venv",
        "env",
        ".env",
        ".pytest_cache",
        "dist",
        "build",
        ".mypy_cache",
        ".tox",
    }

    # 2. 过滤机制：只统计 Python 相关文件
    valid_extensions = {".py", ".pyw"}

    result_files = []

    if not os.path.exists(target_dir):
        print(f"错误: 路径不存在 '{target_dir}'")
        return result_files

    for root, dirs, files in os.walk(target_dir):
        # 原地修改 dirs，跳过隐藏目录和特定的虚拟环境/构建目录
        dirs[:] = [
            d for d in dirs if d not in ignore_dirs and not d.startswith((".", "site-packages"))
        ]

        for file in files:
            ext = os.path.splitext(file)[1].lower()

            if ext not in valid_extensions:
                continue

            file_path = os.path.join(root, file)

            try:
                # 使用 utf-8 读取，忽略掉一些可能存在的非文本字符错误
                with open(file_path, encoding="utf-8", errors="ignore") as f:
                    line_count = sum(1 for _ in f)
                    if line_count >= min_lines:
                        result_files.append((file_path, line_count))
            except Exception as e:
                print(f"读取文件时发生意外错误 {file_path}: {e}")

    # 按行数降序排序
    result_files.sort(key=lambda x: x[1], reverse=True)
    return result_files


if __name__ == "__main__":
    # 修改为你想要扫描的 Python 项目路径
    target_path = r"C:\Computer\my_github\github_cpp\time_tracer\time_tracer_cpp\test"
    # target_path = r"tests"
    threshold = 200

    print(f"正在扫描 Python 项目: {target_path} ...\n")

    if not os.path.exists(target_path):
        print(f"提示：路径不存在，请检查: {target_path}")
    else:
        large_files = analyze_large_python_files(target_path, threshold)

        if large_files:
            print(f"总计找到 {len(large_files)} 个超过 {threshold} 行的 Python 文件：")
            print("-" * 100)
            for file_path, lines in large_files:
                # 保持 File "path" 格式，方便 IDE 终端直接点击跳转
                print(f'{lines:<6} lines | File "{file_path}"')
            print("-" * 100)
        else:
            print(f"扫描完毕，未发现超过 {threshold} 行的文件。")
