# config.py
# 在这里配置你所有的路径和选项

# 1. 源文件夹的父目录
SOURCE_DIRECTORY = "C:/Computer/my_github/github_cpp/bill_master/Bills_Master_cpp/my_test/exported_files"

# 2. 统一的输出目录
OUTPUT_DIRECTORY = "output_pdf"

# 3. 指定要编译的文档类型
COMPILE_TYPES = ['Typst', 'TeX']

# --- 新增功能：部分文件编译 ---

# 4. 是否开启部分编译模式 (1 = 是, 0 = 否)
#    - 设置为 1 时，程序将只编译每个文件夹下的前 N 个文件。
#    - 设置为 0 时，程序将编译每个文件夹下的所有文件。
ENABLE_PARTIAL_COMPILE = 1  # <--- 修改这里来开启或关闭此功能

# 5. 在部分编译模式下，每个文件夹要编译的文件数量。
#    - 如果上面的开关开启，这个数字才会生效。
#    - 如果设置为 0，程序会自动按 1 处理。
PARTIAL_COMPILE_COUNT = 1  # <--- 修改这里来指定编译数量