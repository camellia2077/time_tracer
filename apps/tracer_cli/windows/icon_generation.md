# Windows EXE 图标生成说明

本文说明如何基于 SVG 生成并注入 `time_tracer_cli.exe` 的 Windows 图标资源。

> 目录约定说明（2026-03-07）
> - 品牌设计 SVG 与相关参考资源统一存放在 `design/branding/**`
> - Windows CLI 默认图标设计源已经切换到 `bg_indigo_mist_vertical_padding.svg`
> - 如需新增或替换设计稿，应优先放到 `design/branding/exports/**`

## 适用范围

1. 目标应用：
   - `tracer_windows_rust_cli`
2. 生效阶段：
   - `release` 类 profile
   - 例如 `release_bundle`、`release_safe`
3. 构建入口：
   - `python scripts/run.py build ...`

## 默认图标源

1. 默认 SVG：
   - `design/branding/exports/bg_indigo_mist_vertical_padding.svg`
2. 可选透明底 SVG：
   - `design/branding/exports/bg_golden_vertical_padding_transparent.svg`
3. 生成 ICO 位置：
   - `apps/tracer_cli/windows/rust_cli/<build_dir>/resources/time_tracer_cli.ico`
4. 注入方式：
   - Rust `build.rs` 读取 `TT_WINDOWS_CLI_ICON_ICO`，并将图标写入 EXE 资源节

补充说明：

- 上述 SVG 路径是当前仓库状态下的实际默认路径
- 品牌设计参考目录统一为 `design/branding/**`

## 命令示例

### 1) 使用默认 SVG

```powershell
python scripts/run.py build --app tracer_windows_rust_cli --profile release_bundle --build-dir build
```

### 2) 通过命令行指定 SVG

```powershell
python scripts/run.py build --app tracer_windows_rust_cli --profile release_bundle --build-dir build --windows-icon-svg "design/branding/exports/bg_indigo_mist_vertical_padding.svg"
```

### 3) 通过封装脚本指定 SVG

```powershell
bash apps/tracer_cli/windows/scripts/build_rust_from_windows_build.sh --windows-icon-svg "design/branding/exports/bg_indigo_mist_vertical_padding.svg"
```

## 可选覆盖项

1. `--windows-icon-svg <path>`
   - 优先使用本次命令传入的 SVG
2. `TT_WINDOWS_CLI_ICON_ICO=<path>`
   - 直接使用已有 `.ico`
   - 跳过 SVG 转换
3. `TT_WINDOWS_CLI_ICON_SVG=<path>`
   - 当未传 `--windows-icon-svg` 时，作为默认 SVG 覆盖来源

## 生成流程

Windows 图标构建流程如下：

1. 工具链解析默认 SVG 或覆盖参数
2. 调用 `rsvg-convert` 将 SVG 栅格化为 256x256 PNG
3. 生成 `time_tracer_cli.ico`
4. Rust `build.rs` 将 `.ico` 注入到最终 EXE

默认 SVG 解析逻辑位于：

- `scripts/toolchain/commands/cmd_build/windows_icon_resources.py`

## 依赖与故障排查

1. 依赖工具：
   - `rsvg-convert`
2. 如果提示 `rsvg-convert not found`
   - 安装 librsvg
   - 或设置 `TT_WINDOWS_CLI_ICON_ICO` 指向现成的 `.ico`
3. 如果提示 `source SVG icon not found`
   - 检查 `--windows-icon-svg` 路径是否正确
   - 检查默认 SVG 是否仍存在于 `design/branding/exports/`

## 相关文档

- `apps/tracer_cli/windows/README.md`
- `design/branding/README.md`
- `design/branding/windows-cli-icon-svg-application.md`
