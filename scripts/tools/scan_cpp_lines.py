import os


def analyze_large_cpp_files(target_dir, min_lines=350):
    # 1. 过滤机制：需要跳过的无关/生成目录
    ignore_dirs = {".git", ".idea", ".vscode", "vs", "bin", "obj", "Debug", "Release"}
    # 2. 过滤机制：只统计 C++ 相关的代码文件类型
    valid_extensions = {".cpp", ".h", ".hpp", ".cc", ".cxx", ".c"}

    result_files = []

    if not os.path.exists(target_dir):
        print(f"错误: 路径不存在 '{target_dir}'")
        return result_files

    for root, dirs, files in os.walk(target_dir):
        # 原地修改 dirs 以跳过构建目录
        dirs[:] = [
            d
            for d in dirs
            if d not in ignore_dirs and not d.lower().startswith(("build", "cmake", "out"))
        ]

        for file in files:
            # 【修复点】：使用 [1] 获取后缀名，再进行 lower()
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


if __name__ == "__main__":
    target_path = r"C:\Computer\my_github\github_cpp\time_tracer\time_tracer_cpp\apps\time_tracer"
    threshold = 350

    print(f"正在扫描 C++ 项目: {target_path} ...\n")

    if not os.path.exists(target_path):
        print(f"提示：路径不存在，请检查: {target_path}")
    else:
        large_files = analyze_large_cpp_files(target_path, threshold)

        if large_files:
            print(f"总计找到 {len(large_files)} 个超过 {threshold} 行的代码文件：")
            print("-" * 100)
            for file_path, lines in large_files:
                # 使用标准的 File "path" 格式，方便在 VS Code/PyCharm 控制台直接点击跳转
                print(f'{lines:<6} lines | File "{file_path}"')
            print("-" * 100)
        else:
            print(f"扫描完毕，未发现超过 {threshold} 行的文件。")
