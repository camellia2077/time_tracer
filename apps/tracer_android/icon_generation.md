# Android Icon 生成与替换流程

本文用于说明如何在本项目中更新 Android launcher icon，并与 `design/branding/**` 下的设计源保持一致。

> 目录约定说明（2026-03-07）
> - 品牌设计参考资源统一放在 `design/branding/**`
> - Android 不直接消费 SVG，而是手动维护 `vector drawable` XML
> - 设计稿更新后，需要同步修改 Android 图标资源

## 1. 当前默认设计源

当前 Android 启动图标默认对齐以下设计源：

- `design/branding/exports/bg_golden_vertical_padding_shadow.svg`

可选参考：

- `design/branding/exports/bg_golden_vertical_padding_transparent.svg`

说明：

- Android launcher 当前使用 `vector drawable`，不是直接引用 SVG
- SVG 中的阴影滤镜不会被 Android `vector drawable` 原生支持
- 因此当前实现采用“以 SVG 为视觉基准，再在 XML 中做近似还原”的方式
- `bg_golden_vertical_padding_shadow.svg` 中的阴影，在 Android 端通过半透明偏移图形近似模拟

## 2. 需要修改的文件

Android launcher 资源入口：

- `apps/tracer_android/app/src/main/res/drawable/ic_launcher_foreground.xml`
- `apps/tracer_android/app/src/main/res/drawable/ic_launcher_monochrome.xml`
- `apps/tracer_android/app/src/main/res/values/ic_launcher_colors.xml`
- `apps/tracer_android/app/src/main/res/mipmap-anydpi-v26/ic_launcher.xml`
- `apps/tracer_android/app/src/main/res/mipmap-anydpi-v26/ic_launcher_round.xml`

约定：

- `ic_launcher.xml` 与 `ic_launcher_round.xml` 通常不改结构，只确认仍引用：
  - `@color/ic_launcher_background`
  - `@drawable/ic_launcher_foreground`
  - `@drawable/ic_launcher_monochrome`
- 主要改动集中在：
  - `ic_launcher_foreground.xml`：彩色前景
  - `ic_launcher_monochrome.xml`：单色图标
  - `ic_launcher_colors.xml`：背景色

## 3. 当前默认方案

推荐默认组合如下：

- 视觉源：`design/branding/exports/bg_golden_vertical_padding_shadow.svg`
- Android 前景：`ic_launcher_foreground.xml` 使用白色圆角底 + 金紫渐变箭头
- Android 单色：`ic_launcher_monochrome.xml` 保留箭头与轨迹的单色版本
- Android 背景：`ic_launcher_background = #00000000`（透明）

## 4. 标准操作步骤

1. 确认设计源 SVG
   - 默认使用 `bg_golden_vertical_padding_shadow.svg`
2. 编辑 `ic_launcher_foreground.xml`
   - 同步白色圆角底、箭头、轨迹形状
   - 如果 SVG 含阴影，需要在 XML 中手动做近似模拟
3. 编辑 `ic_launcher_monochrome.xml`
   - 保持几何形状与彩色版本一致
4. 检查 `ic_launcher_colors.xml`
   - 如果前景已经包含白底，背景建议保持透明：`#00000000`
5. 执行定向构建验证：

```bash
python scripts/run.py build --app tracer_android --profile android_edit
```

6. 在设备或模拟器上确认图标显示效果
   - 检查浅色/深色桌面
   - 检查圆形裁切下是否仍居中、无被切边

## 5. 常见问题

1. 改了 XML 但图标没变化
   - 清理安装包后重新安装
   - 某些桌面会缓存图标，可重启启动器后再看

2. SVG 有阴影，但 Android 看起来不完全一样
   - 这是正常现象
   - Android `vector drawable` 不支持 SVG 的 `filter/feDropShadow`
   - 只能使用额外 path 做近似模拟

3. 图标边缘被裁切
   - 检查 `viewportWidth/viewportHeight`
   - 检查路径是否贴边，保留足够安全边距

4. 设计源 SVG 已更新，但 Android 未同步
   - Android 不会自动读取 SVG
   - 必须手动更新 `ic_launcher_foreground.xml` 与 `ic_launcher_monochrome.xml`

## 6. 与其他端的关系

- Windows 图标流程见：`apps/tracer_cli/windows/icon_generation.md`
- Android 与 Windows 共用同一套设计参考目录：`design/branding/**`
- 两端产物格式不同：
  - Windows：由工具链将 SVG 转为 `.ico`
  - Android：手动维护 `vector drawable` XML
