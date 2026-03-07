# Windows CLI Icon Source Mapping

Windows CLI 图标使用 `design/branding/master/time_tracer_brand_master_symbol.svg` 作为品牌母版参考。

## 当前默认构建源

- `design/branding/exports/bg_golden_vertical_padding.svg`

## 说明

- Windows CLI 可以更直接地贴近无背景品牌母版
- 白底与整体画布关系由 Windows 平台导出版决定
- 当前默认构建源保留了你选定的箭头大小与位置
- 构建期会将 SVG 转成 `.ico` 并注入 EXE
- 默认解析逻辑位于：
  - `scripts/toolchain/commands/cmd_build/windows_icon_resources.py`

## 相关文档

- `design/branding/windows-cli-icon-svg-application.md`
