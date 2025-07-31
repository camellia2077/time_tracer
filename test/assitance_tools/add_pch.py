import os
import argparse

def add_pch_include_to_files(directory):
    """
    递归遍历指定目录，为所有 .cpp 文件在第一行添加 #include "pch.h"。
    如果该文件已经包含了此行，则跳过。
    
    Args:
        directory (str): 要处理的项目根目录路径。
    """
    print(f"🚀 开始扫描目录: {directory}")
    
    # 使用 os.walk 递归遍历所有文件和子目录
    cpp_files_found = []
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith('.cpp'):
                cpp_files_found.append(os.path.join(root, file))

    if not cpp_files_found:
        print("🤷 在该目录下没有找到任何 .cpp 文件。")
        return

    print(f"🔍 共找到 {len(cpp_files_found)} 个 .cpp 文件。开始处理...\n")

    modified_count = 0
    skipped_count = 0
    
    # 遍历所有找到的 .cpp 文件
    for file_path in cpp_files_found:
        try:
            with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.readlines() # 按行读取文件内容

            # 检查第一行是否已经是 #include "pch.h"
            if content and content[0].strip() == '#include "common/pch.h"':
                print(f"🟡 已跳过 (已存在): {file_path}")
                skipped_count += 1
                continue

            # 在内容列表的开头插入新的 include 指令
            content.insert(0, '#include "common/pch.h"\n')
            
            # 将修改后的内容写回文件
            with open(file_path, 'w', encoding='utf-8') as f:
                f.writelines(content)
            
            print(f"✅ 已修改: {file_path}")
            modified_count += 1

        except Exception as e:
            print(f"❌ 处理文件时出错: {file_path} - {e}")

    # 打印最终的总结报告
    print("\n" + "="*50)
    print("🎉 处理完成！")
    print(f"  - {modified_count} 个文件被成功修改。")
    print(f"  - {skipped_count} 个文件因已包含该头文件而被跳过。")
    print("="*50)


def main():
    """
    主函数，用于解析命令行参数。
    """
    parser = argparse.ArgumentParser(
        description="一个为C++项目所有.cpp文件自动添加 #include \"pch.h\" 的工具。"
    )
    parser.add_argument(
        "-p", "--path",
        required=True,
        help="【必需】要处理的项目文件夹的根目录路径。"
    )
    
    args = parser.parse_args()
    
    if not os.path.isdir(args.path):
        print(f"❌ 错误: 路径 '{args.path}' 不是一个有效的目录。")
        return
        
    add_pch_include_to_files(args.path)

if __name__ == "__main__":
    main()