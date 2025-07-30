# internal/compilers.py
from typing import List

def build_tex_command(input_path, _, target_dir):
    """构建 TeX 编译命令。"""
    return ['xelatex', '-interaction=nonstopmode', f'-output-directory={target_dir}', input_path]

def build_typ_command(input_path, output_path, _):
    """构建 Typst 编译命令。"""
    return ['typst', 'compile', input_path, output_path]

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