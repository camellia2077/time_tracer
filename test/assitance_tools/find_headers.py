import os
import re
import argparse
from collections import Counter
import pprint
# 查看这个目录下前10被引用的头文件
# py c:\Computer\my_github\github_cpp\bill_master\find_headers.py -p C:\Computer\my_github\github_cpp\bill_master\Bills_Master_cpp\Bills_Master\query -n 10
# 
def find_project_files(directory, extensions=('.cpp', '.h', '.hpp')):
    """
    递归遍历指定目录，查找所有指定扩展名的源文件。

    Args:
        directory (str): 要搜索的根目录路径。
        extensions (tuple): 包含文件扩展名的元组。

    Returns:
        list: 包含所有找到的文件绝对路径的列表。
    """
    project_files = []
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith(extensions):
                project_files.append(os.path.join(root, file))
    return project_files

def analyze_headers(files):
    """
    分析文件列表，统计所有 #include 指令中头文件的出现频率。

    Args:
        files (list): 要分析的文件路径列表。

    Returns:
        collections.Counter: 包含头文件及其出现次数的 Counter 对象。
    """
    # 正则表达式用于匹配 #include 指令并提取头文件名
    # 支持 <header.h> 和 "header.h" 两种形式
    include_pattern = re.compile(r'#\s*include\s*[<"]([^>"]+)[>"]')
    
    header_counts = Counter()
    
    print(f"\n🔍 开始分析 {len(files)} 个文件...")
    
    for i, file_path in enumerate(files, 1):
        try:
            with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
                # 查找所有匹配的头文件
                matches = include_pattern.findall(content)
                # 使用 Counter 一次性更新所有找到的头文件的计数
                header_counts.update(matches)
        except Exception as e:
            print(f"  ❌ 读取文件时出错: {file_path} - {e}")
            
        # 打印进度
        if i % 10 == 0 or i == len(files):
            print(f"  ➡️  已处理 {i}/{len(files)}...", end='\r')

    print("\n✅ 分析完成！")
    return header_counts

def main():
    """
    主函数，负责解析命令行参数并协调整个分析流程。
    """
    # --- 设置命令行参数解析 ---
    parser = argparse.ArgumentParser(
        description="一个用于分析C++项目中头文件使用频率的辅助工具。",
        formatter_class=argparse.RawTextHelpFormatter # 保持帮助文本的格式
    )
    parser.add_argument(
        "-p", "--path",
        required=True,
        help="【必需】指定要分析的项目文件夹的根目录路径。"
    )
    parser.add_argument(
        "-n", "--top",
        type=int,
        default=20, # 默认显示前20个最常用的头文件
        help="指定要显示的最常用头文件的数量 (默认为 20)。"
    )
    
    args = parser.parse_args()
    
    # 验证路径是否存在
    if not os.path.isdir(args.path):
        print(f"❌ 错误: 提供的路径 '{args.path}' 不是一个有效的目录。")
        return

    # --- 执行分析 ---
    # 1. 查找所有 .cpp 和 .h 文件
    project_files = find_project_files(args.path)
    
    if not project_files:
        print(f"⚠️ 在目录 '{args.path}' 中没有找到任何 .cpp 或 .h 文件。")
        return
        
    # 2. 分析头文件并统计频率
    header_counts = analyze_headers(project_files)
    
    if not header_counts:
        print("🤷 未能从文件中提取任何头文件信息。")
        return

    # --- 显示结果 ---
    print("\n" + "="*50)
    print(f"📊 C++ 项目头文件使用频率分析报告 (Top {args.top})")
    print("="*50)
    
    # 3. 按频率从高到低排序，并打印结果
    # most_common(n) 方法可以直接返回频率最高的前 n 个元素
    for header, count in header_counts.most_common(args.top):
        print(f"  - {header:<40} | 使用次数: {count}")
        
    print("\n💡 建议将以上列表中最常用的、且不经常变动的头文件（尤其是标准库和第三方库）放入预编译头文件 (pch.h) 中以提升编译速度。")


if __name__ == "__main__":
    main()