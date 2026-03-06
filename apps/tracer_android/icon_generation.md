# Android Icon 生成与替换流程

本文用于说明如何在本项目中为 Android 应用切换/更新 launcher icon，并与 `design/branding` 下的品牌 SVG 保持一致。

> 目录语义说明（2026-03-06）
>
> - 设计参考 SVG 与品牌探索稿已迁移到 `design/branding/**`。
> - 旧的共享设计目录已退役删除，不再承担共享设计资产归档职责。
> - 如需新增设计稿，请直接放入 `design/branding/**`。

## 1. 设计源文件

当前默认设计源：

- `design/branding/exports/sharp_rounded_white_golden.svg`（推荐，圆角白底）

可选设计源：

- `design/branding/exports/bg_golden_vertical_padding_transparent.svg`（透明底）

说明：

- Android 当前 launcher 使用 `vector drawable`（XML）而不是直接引用 SVG。
- 因此流程是“以 SVG 为视觉基准”，将形状同步到 `ic_launcher_foreground.xml`。

## 2. 需要修改的文件

Android launcher 资源入口：

- `apps/tracer_android/app/src/main/res/drawable/ic_launcher_foreground.xml`
- `apps/tracer_android/app/src/main/res/values/ic_launcher_colors.xml`
- `apps/tracer_android/app/src/main/res/mipmap-anydpi-v26/ic_launcher.xml`
- `apps/tracer_android/app/src/main/res/mipmap-anydpi-v26/ic_launcher_round.xml`

约定：

- `ic_launcher.xml` 与 `ic_launcher_round.xml` 通常不改结构，只确认仍引用：
  - `@color/ic_launcher_background`
  - `@drawable/ic_launcher_foreground`
- 主要改动集中在 `ic_launcher_foreground.xml`（前景图形）和 `ic_launcher_colors.xml`（背景色）。

## 3. 标准操作步骤

1. 选择视觉版本（默认建议 `sharp_rounded_white_golden.svg`）。
2. 编辑 `ic_launcher_foreground.xml`，同步箭头与轨迹形状。
3. 编辑 `ic_launcher_colors.xml`：
   - 若前景图内已经包含白色圆角底，建议背景使用透明：`#00000000`
   - 若前景图仅保留箭头，背景色可改为品牌底色
4. 执行构建验证：

```bash
python "scripts/run.py" build --app tracer_android --profile android_style
```

5. 在设备或模拟器确认桌面图标显示效果（浅色/深色桌面各检查一次）。

## 4. 推荐默认方案

为与 Windows CLI 图标统一，推荐以下默认组合：

- 视觉源：`design/branding/exports/sharp_rounded_white_golden.svg`
- Android 前景：`ic_launcher_foreground.xml` 使用圆角白底 + 箭头
- Android 背景：`ic_launcher_background = #00000000`（透明）

## 5. 常见问题

1. 改了 XML 但图标没变化  
   - 清理并重装应用，或卸载后安装。
   - 部分桌面会缓存图标，切换桌面/重启启动器后再看。

2. 图标边缘被裁切  
   - 检查 `viewportWidth/viewportHeight` 与路径边距，避免贴边。

3. 品牌源 SVG 已更新但 Android 未同步  
   - Android 不会自动读取 SVG，必须手动更新 `ic_launcher_foreground.xml`。

## 6. 与 Windows 流程的关系

- Windows 图标生成流程见：`apps/tracer_cli/windows/icon_generation.md`
- Android 与 Windows 当前共用同一设计源目录：`design/branding/**`
- 两端产物格式不同：
  - Windows：由 Python 构建链将 SVG 转 `.ico`
  - Android：手工维护 vector drawable XML
