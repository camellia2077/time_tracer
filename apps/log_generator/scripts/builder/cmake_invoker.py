# builder/cmake_invoker.py

import os
import re
import subprocess
from pathlib import Path
from typing import List

from .ui.console import print_header
from .ui.colors import AnsiColors


def run_cmake(
    should_package: bool,
    cmake_args: List[str],
    compiler: str,
    config,
    logger,
    no_opt=False,
    no_tidy=False,
    no_lto=False,
    no_warn=False,
    no_pch=False,
    fail_fast=False,
    quiet=False,
):
    """
    Execute CMake configuration.
    """
    print_header("Configuring project with CMake...")

    cmake_env = os.environ.copy()
    if compiler == "gcc":
        cmake_env["CC"] = "gcc"
        cmake_env["CXX"] = "g++"
    elif compiler == "clang":
        cmake_env["CC"] = "clang"
        cmake_env["CXX"] = "clang++"

    cmake_command = ["cmake", "-S", "..", "-B", ".", "-D", "CMAKE_BUILD_TYPE=Release"]

    cmake_command.append(f"-DENABLE_OPTIMIZATION={'OFF' if no_opt else 'ON'}")
    cmake_command.append(f"-DENABLE_CLANG_TIDY={'OFF' if no_tidy else 'ON'}")
    cmake_command.append(f"-DENABLE_PCH={'OFF' if no_pch else 'ON'}")

    warning_level = 0 if no_warn else getattr(config, "WARNING_LEVEL", 2)
    cmake_command.append(f"-DWARNING_LEVEL={warning_level}")

    enable_lto = getattr(config, "ENABLE_LTO", True)
    lto_option = "OFF" if no_lto else ("ON" if enable_lto else "OFF")
    cmake_command.append(f"-DENABLE_LTO={lto_option}")

    cmake_command.append(f"-DTIDY_WARNINGS_AS_ERRORS={'ON' if fail_fast else 'OFF'}")

    cmake_command.extend(cmake_args)

    process = subprocess.Popen(
        cmake_command,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        env=cmake_env,
        encoding="utf-8",
        errors="replace",
    )

    if quiet:
        stdout_captured, stderr_captured = process.communicate()
        logger.log(stdout_captured, end="")
        logger.log_error(stderr_captured, end="")

        if process.returncode != 0:
            print(stdout_captured)
            raise subprocess.CalledProcessError(
                process.returncode, cmake_command, output=stdout_captured, stderr=stderr_captured
            )
    else:
        for line in process.stdout:
            logger.log(line, end="")

        stderr_output = ""
        for line in process.stderr:
            stderr_output += line
            logger.log_error(line, end="")

        process.wait()
        if process.returncode != 0:
            raise subprocess.CalledProcessError(process.returncode, cmake_command, output=None, stderr=stderr_output)

    print("--- CMake configuration complete.")


def run_build(logger, target=None, quiet=False):
    """
    Execute build.

    Returns:
        tuple: (log_lines, success)
    """
    build_cmd = ["cmake", "--build", "."]
    if target:
        build_cmd.extend(["--target", target])
        print_header(f"Running build target: {target}...")
    else:
        print_header("Building...")

    process = subprocess.Popen(
        build_cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        encoding="utf-8",
        errors="replace",
    )

    full_log = []

    if quiet:
        for line in process.stdout:
            full_log.append(line)
            logger.log(line, end="")
    else:
        for line in process.stdout:
            full_log.append(line)
            if re.search(r"error:", line, re.IGNORECASE):
                logger.log(f"{AnsiColors.FAIL}{line}{AnsiColors.ENDC}", end="")
            elif re.search(r"warning:", line, re.IGNORECASE):
                logger.log(f"{AnsiColors.WARNING}{line}{AnsiColors.ENDC}", end="")
            else:
                logger.log(line, end="")

    process.wait()
    success = process.returncode == 0
    if success:
        print("--- Build complete.")
    else:
        print(f"--- Build finished with exit code {process.returncode}.")
    return full_log, success


def run_cpack(logger):
    """
    Execute CPack for installer creation.
    """
    print_header("Creating the installation package with CPack...")
    subprocess.run(["cpack"], check=True, capture_output=True, text=True)

    installers = list(Path.cwd().glob("LogGenerator-*-win64.exe"))
    if not installers:
        raise FileNotFoundError("CPack finished, but no installer was found.")
    return installers[0]
