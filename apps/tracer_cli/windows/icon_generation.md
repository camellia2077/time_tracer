# Windows EXE 图标生成说明

本文档说明如何基于 SVG 生成并注入 `time_tracer_cli.exe` 的 Windows 图标资源。

> 目录语义说明（2026-03-06）
>
> - 设计参考 SVG 与品牌探索稿已迁移到 `design/branding/**`。
> - 旧的共享设计目录已退役删除，不再作为共享设计资产目录存在。
> - 如需新增或替换设计稿，请直接放入 `design/branding/**`。

## 适用范围

1. 目标应用：`tracer_windows_rust_cli`
2. 生效阶段：`release` 类 profile（例如 `release_bundle`、`release_safe`）
3. 入口命令：`python scripts/run.py build ...`

## 默认图标源

1. 默认 SVG（圆角白底）：`design/branding/exports/sharp_rounded_white_golden.svg`
2. 透明底可选 SVG：`design/branding/exports/bg_golden_vertical_padding_transparent.svg`
3. 生成 ICO 位置：`apps/tracer_cli/windows/rust_cli/<build_dir>/resources/time_tracer_cli.ico`
4. 注入方式：Rust `build.rs` 读取 `TT_WINDOWS_CLI_ICON_ICO` 并写入 exe 资源节。

补充：

- 上述 SVG 路径是当前仓库状态下的实际路径。
- 设计参考文件现已统一归档到 `design/branding/**`。

## 命令示例

### 1) 使用默认 SVG（圆角白底）

```bash
python scripts/run.py build --app tracer_windows_rust_cli --profile release_bundle --build-dir build
```

### 2) 通过命令行指定 SVG（推荐）

```bash
python scripts/run.py build --app tracer_windows_rust_cli --profile release_bundle --build-dir build --windows-icon-svg "design/branding/exports/bg_golden_vertical_padding_transparent.svg"
```

### 3) 通过封装脚本指定 SVG

```bash
bash apps/tracer_cli/windows/scripts/build_rust_from_windows_build.sh --windows-icon-svg "design/branding/exports/bg_golden_vertical_padding_transparent.svg"
```

## 可选覆盖项

1. `--windows-icon-svg <path>`  
   - 优先级最高，直接指定本次构建使用的 SVG。
2. `TT_WINDOWS_CLI_ICON_ICO=<path>`  
   - 若设置，直接使用该 `.ico`，跳过 SVG 转换。
3. `TT_WINDOWS_CLI_ICON_SVG=<path>`  
   - 当未传 `--windows-icon-svg` 时，作为 SVG 来源覆盖默认路径。

## 依赖与故障排查

1. 需要 `rsvg-convert`（librsvg）用于 `svg -> png(256x256)`。
2. 若提示 `rsvg-convert not found`：
   - 安装 librsvg，或
   - 直接设置 `TT_WINDOWS_CLI_ICON_ICO` 指向现成 ico。
3. 若提示 `source SVG icon not found`：
   - 检查 `--windows-icon-svg` 路径是否正确。
