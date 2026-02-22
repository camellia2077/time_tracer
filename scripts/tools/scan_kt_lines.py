import os


def analyze_large_files(target_dir, min_lines=350):
    # 1. 过滤机制：需要跳过的无关/生成目录
    ignore_dirs = {".git", ".gradle", "build", ".idea", "captures"}
    # 2. 过滤机制：只统计有意义的代码文件类型
    valid_extensions = {".kt"}

    result_files = []

    # 检查路径是否存在
    if not os.path.exists(target_dir):
        print(f"错误: 路径不存在 '{target_dir}'")
        return result_files

    for root, dirs, files in os.walk(target_dir):
        # 原地修改 dirs 列表，阻止 os.walk 进入不需要遍历的目录（极大提升效率）
        dirs[:] = [d for d in dirs if d not in ignore_dirs]

        for file in files:
            ext = os.path.splitext(file)[1].lower()
            if ext not in valid_extensions:
                continue

            # 因为 target_dir 传入的是绝对路径，此处 os.path.join 生成的 file_path 也就是完整的绝对路径
            file_path = os.path.join(root, file)

            try:
                # 3. 容错处理：使用 utf-8 读取，并忽略无法解析的非常规字符防止崩溃
                with open(file_path, encoding="utf-8", errors="ignore") as f:
                    # 使用生成器进行高效的行数统计，无需将整个文件读入内存
                    line_count = sum(1 for _ in f)

                    if line_count > min_lines:
                        result_files.append((file_path, line_count))
            except Exception as e:
                print(f"读取文件时发生意外错误 {file_path}: {e}")

    # 4. 结果整理：按行数从多到少降序排序，方便查看最臃肿的文件
    result_files.sort(key=lambda x: x[1], reverse=True)
    return result_files


if __name__ == "__main__":
    # 使用 r 前缀处理 Windows 路径中的反斜杠
    target_path = (
        r"C:\Computer\my_github\github_cpp\time_tracer\time_tracer_cpp\apps\tracer_android"
    )
    threshold = 350

    print(f"正在扫描: {target_path} ...\n")
    large_files = analyze_large_files(target_path, threshold)

    if large_files:
        print(f"总计找到 {len(large_files)} 个超过 {threshold} 行的代码文件：")
        print("-" * 80)
        for file_path, lines in large_files:
            # 【修改点】：直接使用 file_path。
            # 格式化为 File "绝对路径" 的形式，可以最大程度兼容各类 IDE 的超链接识别机制
            print(f'[{lines:4} 行] | File "{file_path}"')
        print("-" * 80)
    else:
        print(f"未找到行数大于 {threshold} 行的文件。")
