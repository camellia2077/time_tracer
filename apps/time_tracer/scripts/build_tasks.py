import shutil
from pathlib import Path
from build_config import config
from build_ui import print_header, print_warning

def parse_arguments(args):
    """解析命令行参数"""
    print_header("Parsing command-line arguments...")
    
    # 从配置中获取默认编译器，可以被命令行覆盖
    compiler = config.COMPILER
    
    should_install = 'install' in args
    if should_install:
        args.remove('install')
        should_clean = True
        should_package = True
    else:
        should_clean = 'clean' in args
        should_package = '--package' in args or '-p' in args

    if '--gcc' in args:
        compiler = 'gcc'
        args.remove('--gcc')
        print("--- Compiler override: GCC will be used.")
    elif '--clang' in args:
        compiler = 'clang'
        args.remove('--clang')
        print("--- Compiler override: Clang will be used.")

    cmake_args = [arg for arg in args if arg.startswith('-D')]
    if cmake_args:
        print(f"--- Found manual CMake arguments: {' '.join(cmake_args)}")
    
    known_args = {'clean', '--package', '-p', 'install', '--gcc', '--clang'}
    remaining_args = [arg for arg in args if arg not in known_args and not arg.startswith('-D')]
    
    if remaining_args:
        for arg in remaining_args:
            print_warning(f"Warning: Unknown argument '{arg}' ignored.")
            
    return {"clean": should_clean, "package": should_package, "install": should_install, "compiler": compiler}, cmake_args

def prepare_build_directory(project_dir, build_dir_name, should_clean):
    """创建或清理构建目录"""
    build_dir = project_dir / build_dir_name
    
    if should_clean:
        print_header("'clean' or 'install' provided. Cleaning previous build artifacts...")
        if build_dir.exists():
            shutil.rmtree(build_dir)
        print("--- Cleanup complete.")

    build_dir.mkdir(exist_ok=True)
    return build_dir