# config.py
# 在这里配置你所有的路径和选项

# 1. 源文件夹的父目录
#    程序将扫描这个文件夹下的所有子目录（如 'LaTeX_bills', 'Markdown_bills' 等）。
SOURCE_DIRECTORY = "C:/Computer/my_github/github_cpp/New_time_master/my_test/exported_files_mini"  # <--- 修改这里：指向包含各类bills文件夹的父目录

# 2. 统一的输出目录
#    所有编译好的文件，都会保持其原始目录结构，并存放在这里。
OUTPUT_DIRECTORY = "output_pdf"  # <--- 修改这里：指定你想要的输出文件夹名称

# 3. (新增) 指定要编译的文档类型
#    这是一个列表，只编译列表中指定的类型。
#    可选值: 'TeX', 'Markdown', 'RST', 'Typst'
#    - 如果要编译所有类型，请保留全部: ['TeX', 'Markdown', 'RST', 'Typst']
#    - 如果只想编译 Typst，就改成: ['Typst']
#    - 如果只想编译 Markdown 和 TeX，就改成: ['Markdown', 'TeX']
#    (注意：大小写不敏感，'typst' 和 'Typst' 效果一样)
COMPILE_TYPES = ['Tex',"Markdown"]  # <--- 修改这里：指定你想要编译的类型