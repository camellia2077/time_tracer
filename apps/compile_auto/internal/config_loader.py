# internal/config_loader.py
import sys
from pathlib import Path

# 从 Python 3.11+ 开始，tomllib 是内置模块
if sys.version_info >= (3, 11):
    import tomllib
else:
    try:
        import tomli as tomllib
    except ImportError:
        raise ImportError("For Python versions < 3.11, please install 'tomli' using 'pip install tomli'")

def _load_paths(toml_data, paths_class):
    """从 TOML 数据中加载路径配置。"""
    paths_data = toml_data.get("paths", {})
    paths_class.SOURCE_DIRECTORY = paths_data.get("source_directory", "")
    paths_class.OUTPUT_DIRECTORY = paths_data.get("output_directory", "output_pdf")

def _load_compilation(toml_data, compilation_class):
    """从 TOML 数据中加载编译设置。"""
    comp_data = toml_data.get("compilation", {})
    compilation_class.TYPES = comp_data.get("types", ["Typst"])
    compilation_class.INCREMENTAL = comp_data.get("incremental", True)
    compilation_class.CLEAN_OUTPUT_DEFAULT = comp_data.get("clean_output_default", False)

def _load_benchmark(toml_data, benchmark_class):
    """从 TOML 数据中加载基准测试参数。"""
    bench_data = toml_data.get("benchmark", {})
    benchmark_class.COMPILERS = bench_data.get("compilers", ["pandoc", "typst"])
    benchmark_class.LOOPS = bench_data.get("loops", 3)

def load_config(paths_class, compilation_class, benchmark_class):
    """
    主加载函数，协调所有配置的加载。
    """
    config_path = Path("config.toml")
    if not config_path.exists():
        raise FileNotFoundError(f"配置文件 '{config_path}' 未找到。请创建一个。")

    try:
        with open(config_path, "rb") as f:
            toml_data = tomllib.load(f)

        _load_paths(toml_data, paths_class)
        _load_compilation(toml_data, compilation_class)
        _load_benchmark(toml_data, benchmark_class)

    except Exception as e:
        raise RuntimeError(f"加载 '{config_path}' 时出错: {e}")