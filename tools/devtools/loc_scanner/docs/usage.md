# Usage

## 基本命令

```bash
python -m tools.devtools.loc_scanner --lang <cpp|kt|py|rs> [paths ...] [--over N | --under [N] | --dir-over-files [N]] [--dir-max-depth N] [--log-file <path>]
```

## 示例

```bash
# Python 大文件
python -m tools.devtools.loc_scanner --lang py --over 200

# Kotlin 小文件（使用配置默认阈值）
python -m tools.devtools.loc_scanner --lang kt --under

# C++ 指定目录
python -m tools.devtools.loc_scanner --lang cpp libs --over 300

# 目录文件密度扫描
python -m tools.devtools.loc_scanner --lang py --dir-over-files --dir-max-depth 2
```

## Windows bat 入口

```bat
tools\devtools\loc_scanner\scripts\run_py.bat
tools\devtools\loc_scanner\scripts\run_kt.bat
tools\devtools\loc_scanner\scripts\run_cpp.bat
tools\devtools\loc_scanner\scripts\run_rs.bat
```

可追加参数透传给 `run.py`。

## 配置文件

默认配置文件：`tools/devtools/loc_scanner/config/scan_lines.toml`

配置字段说明：`tools/devtools/loc_scanner/docs/toml_config.md`

