# Windows CLI 图标 SVG 应用说明

本文说明如何将 `ui/branding/exports/**` 下的 SVG 设计稿，应用到 Windows CLI 的 EXE 图标资源。

## 目标

- 明确 Windows CLI 图标的默认设计源
- 说明 SVG 如何在构建期转换为 `.ico`
- 统一后续 Windows 图标更新方式

## 设计源目录

Windows CLI 图标设计源优先从以下目录选择：

- `ui/branding/master/`
- `ui/branding/exports/`

当前默认使用：

- `ui/branding/master/time_tracer_brand_master.svg`

可选参考：

- `ui/branding/exports/bg_full_canvas_light_vertical_padding.svg`
- `ui/branding/exports/bg_golden_vertical_padding_transparent.svg`

## 构建产物位置

Windows CLI 图标在构建期生成：

- `apps/tracer_cli/windows/rust_cli/<build_dir>/resources/time_tracer_cli.ico`

生成后的 `.ico` 会由以下文件注入到 EXE 资源：

- `apps/tracer_cli/windows/rust_cli/build.rs`

## 默认生效方式

Windows CLI 的默认 SVG 来源由工具链代码决定：

- `tools/toolchain/commands/cmd_build/windows_icon_resources.py`

当未显式覆盖时，默认使用：

- `ui/branding/master/time_tracer_brand_master.svg`

## 标准应用流程

### 1. 选定 SVG 设计源

默认选用：

- `ui/branding/master/time_tracer_brand_master.svg`

如果需要沿用旧的满画布浅底版本，可改用：

- `ui/branding/exports/bg_full_canvas_light_vertical_padding.svg`

如果需要透明底变体，可改用：

- `ui/branding/exports/bg_golden_vertical_padding_transparent.svg`

### 2. 执行 Windows CLI release 构建

在仓库根目录执行：

```powershell
python tools/run.py build --app tracer_windows_rust_cli --profile release_bundle --build-dir build --runtime-platform windows
```

构建期会：

- 读取默认 SVG
- 调用 `rsvg-convert` 栅格化为 256x256 PNG
- 生成 `time_tracer_cli.ico`
- 由 Rust `build.rs` 注入到 EXE

### 3. 如需临时覆盖本次构建图标

可使用命令行参数：

```powershell
python tools/run.py build --app tracer_windows_rust_cli --profile release_bundle --build-dir build --runtime-platform windows --windows-icon-svg "ui/branding/exports/bg_golden_vertical_padding_transparent.svg"
```

如需测试其他圆角特调版本，可改为：

```powershell
python tools/run.py build --app tracer_windows_rust_cli --profile release_bundle --build-dir build --runtime-platform windows --windows-icon-svg "ui/branding/exports/bg_white_vertical_padding_rounded.svg"
```

也可使用环境变量：

- `TT_WINDOWS_CLI_ICON_SVG=<path>`
- `TT_WINDOWS_CLI_ICON_ICO=<path>`

优先级说明：

1. `TT_WINDOWS_CLI_ICON_ICO`
2. `--windows-icon-svg`
3. `TT_WINDOWS_CLI_ICON_SVG`
4. 工具链默认值

## 依赖要求

SVG 转 `.ico` 依赖：

- `rsvg-convert`

如果缺少该工具：

- 工具链会尝试回退到已存在的缓存 `.ico`
- 若无缓存，则本次图标生成失败

## 维护约定

- Windows CLI 默认图标设计源当前固定为 `time_tracer_brand_master.svg`
- `bg_white_vertical_padding_rounded.svg` 等文件作为旧版导出稿保留
- 若默认视觉方案改变，应同时更新：
  - `tools/toolchain/commands/cmd_build/windows_icon_resources.py`
  - `apps/tracer_cli/windows/README.md`
  - `apps/tracer_cli/windows/icon_generation.md`
  - `apps/tracer_cli/windows/agent.md`
  - `ui/branding/README.md`

## 相关文档

- `ui/branding/README.md`
- `apps/tracer_cli/windows/icon_generation.md`
