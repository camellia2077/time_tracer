import os


def analyze_large_cpp_files(target_dir, min_lines=350):
    """
    扫描单个目录下符合条件的 C++ 文件
    """
    ignore_dirs = {".git", ".idea", ".vscode", "vs", "bin", "obj", "Debug", "Release"}
    valid_extensions = {".cpp", ".h", ".hpp", ".cc", ".cxx", ".c"}

    result_files = []

    if not os.path.exists(target_dir):
        return None  # 返回 None 表示路径无效

    for root, dirs, files in os.walk(target_dir):
        # 排除无关目录
        dirs[:] = [
            d
            for d in dirs
            if d not in ignore_dirs and not d.lower().startswith(("build", "cmake", "out"))
        ]

        for file in files:
            ext = os.path.splitext(file)[1].lower()
            if ext not in valid_extensions:
                continue

            file_path = os.path.join(root, file)
            try:
                with open(file_path, encoding="utf-8", errors="ignore") as f:
                    line_count = sum(1 for _ in f)
                    if line_count > min_lines:
                        result_files.append((file_path, line_count))
            except Exception as e:
                print(f"读取文件时发生意外错误 {file_path}: {e}")

    # 按行数降序排序
    result_files.sort(key=lambda x: x[1], reverse=True)
    return result_files


def run_batch_analysis(path_list, threshold=350):
    """
    循环处理多个路径并格式化输出
    """
    print(f"{'=' * 100}")
    print(f"C++ 代码行数扫描报告 (阈值: > {threshold} 行)")
    print(f"{'=' * 100}\n")

    for path in path_list:
        # 获取文件夹最后一部分作为项目名
        # rstrip 防止路径末尾有斜杠导致 basename 为空
        project_name = os.path.basename(path.rstrip(os.path.abspath(os.sep)))

        print(f"▶ 正在扫描项目: [{project_name}]")
        print(f"  路径: {path}")

        large_files = analyze_large_cpp_files(path, threshold)

        if large_files is None:
            print("  ❌ 错误: 路径不存在，跳过扫描。\n")
            continue

        if large_files:
            print(f"  找到 {len(large_files)} 个大文件：")
            for file_path, lines in large_files:
                # 依然保持 File "path" 格式方便 IDE 点击跳转
                print(f'  {lines:<6} lines | File "{file_path}"')
        else:
            print(f"  ✅ 扫描完毕，未发现超过 {threshold} 行的文件。")

        print(f"\n{'-' * 100}\n")


if __name__ == "__main__":
    # 1. 在这里配置你需要扫描的多个目录
    target_paths = [
        r"C:\Computer\my_github\github_cpp\time_tracer\time_tracer_cpp\apps\tracer_cli\windows",
        r"C:\Computer\my_github\github_cpp\time_tracer\time_tracer_cpp\apps\tracer_core",
    ]

    # 2. 设置行数阈值
    threshold_value = 350

    # 3. 执行
    run_batch_analysis(target_paths, threshold_value)
