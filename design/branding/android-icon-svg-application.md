# Android 图标 SVG 应用说明

本文说明如何将 `design/branding/exports/**` 下的 SVG 设计稿，应用到 Android launcher icon 资源。

## 目标

- 让 `design/branding` 作为 Android 图标的设计源目录
- 明确 SVG 与 Android 图标资源之间的对应关系
- 统一后续图标更新流程，避免设计稿与实现脱节

## 设计源目录

Android 图标设计源优先从以下目录选择：

- `design/branding/exports/`

当前默认视觉参考使用：

- `design/branding/exports/bg_golden_vertical_padding_shadow.svg`

当前 Android 平台专用参考稿：

- `design/branding/platform/android/time_tracer_android_launcher_reference.svg`

可选参考：

- `design/branding/exports/bg_golden_vertical_padding_transparent.svg`

## Android 图标资源位置

Android 启动图标相关资源位于：

- `apps/tracer_android/app/src/main/res/drawable/ic_launcher_foreground.xml`
- `apps/tracer_android/app/src/main/res/drawable/ic_launcher_monochrome.xml`
- `apps/tracer_android/app/src/main/res/values/ic_launcher_colors.xml`
- `apps/tracer_android/app/src/main/res/mipmap-anydpi-v26/ic_launcher.xml`
- `apps/tracer_android/app/src/main/res/mipmap-anydpi-v26/ic_launcher_round.xml`

## 核心原则

1. `design/branding` 中的 SVG 是设计参考源，不是 Android 直接运行时资源
2. Android launcher icon 当前使用 `vector drawable` XML，而不是直接加载 SVG
3. 因此更新图标时，做法是：
   - 以无背景品牌母版作为识别核心
   - 以 Android 平台专用参考稿控制安全区
   - 再以 SVG 视觉参考做颜色、白底和阴影补偿
   - 手动把形状同步到 Android XML 资源
4. `ic_launcher.xml` 与 `ic_launcher_round.xml` 一般不改结构
5. 主要改动集中在：
   - `ic_launcher_foreground.xml`
   - `ic_launcher_monochrome.xml`
   - `ic_launcher_colors.xml`

## 标准应用流程

### 1. 选定 SVG 设计源

默认视觉参考选用：

- `design/branding/exports/bg_golden_vertical_padding_shadow.svg`

平台安全区参考选用：

- `design/branding/platform/android/time_tracer_android_launcher_reference.svg`

如果只是需要无阴影或透明底参考，也可以改看：

- `design/branding/exports/bg_golden_vertical_padding_transparent.svg`

### 2. 同步彩色前景图标

编辑：

- `apps/tracer_android/app/src/main/res/drawable/ic_launcher_foreground.xml`

需要从 SVG 与平台参考稿同步这些视觉元素：

- 白色圆角底
- 主箭头形状
- 左侧轨迹条
- 中间轨迹条
- 渐变方向与颜色
- 前景整体安全区缩放

### 3. 同步单色图标

编辑：

- `apps/tracer_android/app/src/main/res/drawable/ic_launcher_monochrome.xml`

要求：

- 与彩色版保持一致的几何轮廓
- 仅保留单色表达
- 不引入额外背景色
- 与彩色版保持相同安全区缩放

### 4. 检查背景色

编辑：

- `apps/tracer_android/app/src/main/res/values/ic_launcher_colors.xml`

当前默认方案：

- `ic_launcher_background = #00000000`

原因：

- 白色圆角底已经在前景图里
- 透明背景更适合当前图标结构

### 5. 保持自适应图标入口不变

检查以下文件仍正确引用前景/背景资源：

- `apps/tracer_android/app/src/main/res/mipmap-anydpi-v26/ic_launcher.xml`
- `apps/tracer_android/app/src/main/res/mipmap-anydpi-v26/ic_launcher_round.xml`

通常只需确认它们继续引用：

- `@color/ic_launcher_background`
- `@drawable/ic_launcher_foreground`
- `@drawable/ic_launcher_monochrome`

## 阴影处理说明

`bg_golden_vertical_padding_shadow.svg` 含有阴影效果，但 Android `vector drawable` 不支持 SVG 的 `filter/feDropShadow`。

因此 Android 端应采用近似方案：

- 在 `ic_launcher_foreground.xml` 中增加半透明偏移图形
- 用视觉近似代替真实 SVG 阴影滤镜

这意味着：

- Android 图标会尽量接近设计稿
- 但不会与 SVG 渲染结果做到像素级完全一致

## 验证方式

在仓库根目录执行：

```powershell
python scripts/run.py build --app tracer_android --profile android_edit
```

需要确认：

- 构建成功
- 图标未被裁切
- 深浅色桌面显示正常
- 单色图标在系统场景下可读

## 维护约定

- 新的 Android 图标设计稿优先放到 `design/branding/exports/`
- 不要把 Android 专用实现细节写回 SVG 目录
- SVG 负责设计源，Android XML 负责平台落地
- 当品牌母版、Android 平台参考稿或 SVG 视觉参考改版时，必须同步更新 Android 图标资源

## 相关文档

- `design/branding/README.md`
- `apps/tracer_android/icon_generation.md`
