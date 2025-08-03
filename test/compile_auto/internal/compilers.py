# internal/compilers.py
import os
from typing import List,Any

def build_tex_command(input_path, _, target_dir):
    """构建 TeX 编译命令。"""
    return ['xelatex', '-interaction=nonstopmode', f'-output-directory={target_dir}', input_path]

def build_typ_command(input_path, output_path, _):
    """构建 Typst 编译命令。"""
    return ['typst', 'compile', input_path, output_path]

def get_typst_template_content(font: str = "Noto Serif SC") -> str:
    """
    (新增) 生成 Typst 模板的内容字符串。
    """
    return f'#set text(font: "{font}")\n$body$'

# --- 修改后函数 ---
def build_md_to_typ_command(input_path: str, output_path: str, template_path: str) -> List[str]:
    """
    (修改后) 构建使用指定模板路径的 Pandoc (md -> typ) 转换命令。
    此版本不再创建文件，只负责构建命令，依赖外部传入模板路径。
    """
    return [
        'pandoc',
        '--from=gfm',
        '-t', 'typst',
        f'--template={template_path}',
        input_path,
        '-o', output_path
    ]

class PandocCommandBuilder:
    """为需要额外配置（如字体）的 Pandoc 命令构建一个可序列化的类。"""
    def __init__(self, source_format: str, font: str):
        self.source_format = source_format
        self.font = font

    def __call__(self, input_path: str, output_path: str, _) -> List[str]:
        return [
            'pandoc',
            f'--from={self.source_format}',
            input_path,
            '-o',
            output_path,
            '--pdf-engine=xelatex',
            '-V', f'mainfont={self.font}',
            '-V', 'lang=zh-CN',
            '-V', 'geometry:margin=1in'
        ]