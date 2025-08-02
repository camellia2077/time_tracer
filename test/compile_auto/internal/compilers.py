# internal/compilers.py
import os
from typing import List,Any

def build_tex_command(input_path, _, target_dir):
    """构建 TeX 编译命令。"""
    return ['xelatex', '-interaction=nonstopmode', f'-output-directory={target_dir}', input_path]

def build_typ_command(input_path, output_path, _):
    """构建 Typst 编译命令。"""
    return ['typst', 'compile', input_path, output_path]

def build_md_to_typ_command(input_path: str, output_path: str, _: Any, font: str = "Noto Serif SC") -> List[str]:
    """
    构建 Pandoc (md -> typ) 转换命令。
    (新增) 此函数现在会动态创建一个包含字体设置的模板。
    """
    template_content = f'#set text(font: "{font}")\n$body$'
    template_path = os.path.join(os.path.dirname(output_path), "typst_template.typ")
    
    # 写入模板文件
    try:
        with open(template_path, 'w', encoding='utf-8') as f:
            f.write(template_content)
    except IOError as e:
        print(f"警告：无法写入 Typst 模板文件 '{template_path}': {e}")
        # 如果无法写入模板，则回退到不带模板的命令
        return ['pandoc', f'--from=gfm', '-t', 'typst', input_path, '-o', output_path]

    return [
        'pandoc',
        f'--from=gfm',
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