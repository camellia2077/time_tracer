#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import subprocess
import sys
import time
from pathlib import Path
from build_config import config
import build_tasks

def main():
    """主函数，负责编排整个构建流程。"""
    start_time = time.monotonic()
    
    project_dir = Path.cwd()
    build_tasks.print_header(f"Working in project directory: {project_dir}")

    build_dir_name = "build"
    installer_file = None
    
    try:
        options, cmake_args = build_tasks.parse_arguments(sys.argv[1:])
        build_dir = build_tasks.prepare_build_directory(project_dir, build_dir_name, options["clean"])
        
        os.chdir(build_dir)
        
        build_tasks.run_cmake(options["package"], cmake_args)
        build_tasks.run_build()

        if options["package"]:
            installer_file = build_tasks.run_cpack()
        if options["install"]:
            build_tasks.run_installer(installer_file)

    except subprocess.CalledProcessError as e:
        print(f"\n{config.FAIL}!!! A build step failed with exit code {e.returncode}.{config.ENDC}")
        if e.stdout: print(f"{config.FAIL}Output:\n{e.stdout}{config.ENDC}")
        if e.stderr: print(f"{config.FAIL}Error output:\n{e.stderr}{config.ENDC}")
        sys.exit(e.returncode)
    except Exception as e:
        print(f"\n{config.FAIL}!!! An unexpected error occurred: {e}{config.ENDC}")
        sys.exit(1)
    finally:
        end_time = time.monotonic()
        duration = int(end_time - start_time)
        minutes, seconds = divmod(duration, 60)

        print("\n" + "="*60)
        print(f"{config.OKGREEN}{config.BOLD}Process finished successfully!{config.ENDC}")
        print(f"Artifacts are in the '{build_dir_name}' directory.")
        
        # 重新解析以获取最终状态，用于打印准确的消息
        options, _ = build_tasks.parse_arguments(sys.argv[1:]) 
        if options["package"] and not options["install"]:
            print("Installation package has also been created.")
        if options["install"]:
            print("Project has been built, packaged, and the installer was launched.")
        
        print("-" * 60)
        print(f"Total time elapsed: {config.BOLD}{minutes}m {seconds}s{config.ENDC}")
        print("="*60)

if __name__ == "__main__":
    main()