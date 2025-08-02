# config.py
# 在这里配置你所有的路径和选项

# 1. 源文件夹的父目录
SOURCE_DIRECTORY = "C:/Computer/my_github/github_cpp/New_time_master/my_test/exported_files"

# 2. 统一的输出目录
OUTPUT_DIRECTORY = "output_pdf"

# 3. 指定要编译的文档类型
# 可选值: 'TeX', 'Markdown', 'RST', 'Typst'
# time_master暂时还没有实现rst，只有'TeX', 'Markdown', 'Typst'
# bill_master 'TeX', 'Markdown', 'RST', 'Typst'都实现了
COMPILE_TYPES = ['TeX', 'Markdown', 'Typst']

# --- 以下为 Markdown 基准测试配置 ---

# 4. Markdown 编译方式 (列表)
#  不推荐使用pandoc 因为编译速度实在是太慢了，选择typst会比pandoc快大概三倍
#    - 如果列表只有一个值 (如 ['pandoc'])，则执行常规编译。
#    - 如果列表有多个值 (如 ['pandoc', 'typst'])，则启动基准测试模式。
#    可选值: 'pandoc', 'typst'
MARKDOWN_COMPILERS = ['typst']

# 5. 基准测试循环次数
#    仅在基准测试模式下生效。
BENCHMARK_LOOPS = 3