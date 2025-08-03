# config.py
# 在这里配置你所有的路径和选项

# 1. 源文件夹的父目录
SOURCE_DIRECTORY = r"C:\Computer\my_github\github_cpp\New_time_master\my_test\exported_files"

# 2. 统一的输出目录
OUTPUT_DIRECTORY = "output_pdf"

# 3. 指定要编译的文档类型
# bill_master可选值: 'TeX', 'Markdown', 'RST', 'Typst'
# time_master可选值: 'TeX', 'Markdown', 'Typst'
COMPILE_TYPES = ['TeX', 'Markdown', 'RST', 'Typst']


# === 增量编译设置 ===
# True:  启用增量编译 (推荐)。只编译有变动的文件。
# False: 禁用增量编译。每次都重新编译所有文件。
INCREMENTAL_COMPILE = True
# ========================


# === 新增: 清理设置 ===
# True:  每次启动时，默认清理 output_pdf 目录。
# False: 每次启动时，默认不清理。(推荐，以便使用增量编译)
# 这个设置可以被命令行的 --clean 参数覆盖。
CLEAN_OUTPUT_DEFAULT = False
# ========================


# --- 以下为 Markdown 基准测试配置 ---
# ... (后续内容保持不变) ...
MARKDOWN_COMPILERS = ['typst']
BENCHMARK_LOOPS = 3