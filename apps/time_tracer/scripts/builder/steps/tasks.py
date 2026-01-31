# builder/steps/tasks.py

import shutil
from pathlib import Path
from ..ui.console import print_header, print_warning

def parse_arguments(args, default_compiler='default'):
    """
    解析命令行参数
    :param args: 命令行参数列表 (通常是 sys.argv[1:])
    :param default_compiler: 从配置中读取的默认编译器
    :return: (options_dict, cmake_args_list)
    """
    print_header("Parsing command-line arguments...")
    
    # 使用传入的默认编译器
    compiler = default_compiler
    
    # --- 处理 install 参数 ---
    # 如果指定了 install，隐含 clean 和 package 操作
    should_install = 'install' in args
    if should_install:
        args.remove('install')
        should_clean = True
        should_package = True
    else:
        should_clean = 'clean' in args
        should_package = '--package' in args or '-p' in args

    # --- 处理编译器覆盖 ---
    if '--gcc' in args:
        compiler = 'gcc'
        args.remove('--gcc')
        print("--- Compiler override: GCC will be used.")
    elif '--clang' in args:
        compiler = 'clang'
        args.remove('--clang')
        print("--- Compiler override: Clang will be used.")

    # --- 处理优化开关 ---
    should_no_opt = '--no-opt' in args
    if should_no_opt:
        args.remove('--no-opt')
        print("--- Optimization override: Optimizations will be disabled for faster compilation.")

    # --- [新增] 处理 Clang-Tidy 开关 ---
    should_no_tidy = '--no-tidy' in args
    if should_no_tidy:
        args.remove('--no-tidy')
        print("--- Static Analysis override: Clang-Tidy will be disabled.")

    # --- 提取 CMake 参数 (-D 开头) ---
    cmake_args = [arg for arg in args if arg.startswith('-D')]
    if cmake_args:
        print(f"--- Found manual CMake arguments: {' '.join(cmake_args)}")
    
    # --- 检查是否有未知参数 ---
    # [修改] 将 --no-tidy 加入已知参数列表
    known_args = {'clean', '--package', '-p', 'install', '--gcc', '--clang', '--no-opt', '--no-tidy'}
    # 过滤掉已知参数和 CMake 参数，剩下的就是未知的
    remaining_args = [arg for arg in args if arg not in known_args and not arg.startswith('-D')]
    
    if remaining_args:
        for arg in remaining_args:
            print_warning(f"Warning: Unknown argument '{arg}' ignored.")
            
    return {
        "clean": should_clean, 
        "package": should_package, 
        "install": should_install, 
        "compiler": compiler,
        "no_opt": should_no_opt,
        "no_tidy": should_no_tidy  # [新增] 返回 no_tidy 状态
    }, cmake_args

def prepare_build_directory(project_dir: Path, build_dir_name: str, should_clean: bool):
    """
    创建或清理构建目录
    :param project_dir: 项目根目录 (Path 对象)
    :param build_dir_name: 构建目录名称 (如 "build")
    :param should_clean: 是否执行清理
    """
    build_dir = project_dir / build_dir_name
    
    if should_clean:
        print_header("'clean' or 'install' provided. Cleaning previous build artifacts...")
        if build_dir.exists():
            shutil.rmtree(build_dir)
        print("--- Cleanup complete.")

    build_dir.mkdir(exist_ok=True)
    return build_dir