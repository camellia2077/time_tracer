# Android 图标 SVG 应用说明

本文说明如何将 `design/branding` 下的品牌母版与平台参考稿，落地为 Android launcher icon 资源。

## 目标

- 让 `design/branding/master/` 作为品牌识别核心来源
- 让 `design/branding/platform/android/` 作为 Android 平台适配来源
- 让 Android 最终资源遵循标准 adaptive icon 的 `background / foreground / monochrome` 分层

## 设计源层级

Android 图标当前采用两级设计源：

1. 品牌母版：
   - `design/branding/master/time_tracer_brand_master_symbol.svg`
2. Android 平台参考稿：
   - `design/branding/platform/android/time_tracer_android_launcher_reference.svg`

说明：

- 品牌母版只保留箭头与轨迹符号，不写死背景和 safe zone
- Android 平台参考稿负责定义：
  - `Indigo Mist` full-bleed 浅品牌色背景层
  - 前景符号大小
  - 与 adaptive icon 对应的安全区边距
  - 最终由 launcher mask 呈现的外轮廓预期

## Android 图标资源位置

Android 启动图标相关资源位于：

- `apps/tracer_android/app/src/main/res/drawable/ic_launcher_background.xml`
- `apps/tracer_android/app/src/main/res/drawable/ic_launcher_foreground.xml`
- `apps/tracer_android/app/src/main/res/drawable/ic_launcher_foreground_symbol.xml`
- `apps/tracer_android/app/src/main/res/drawable/ic_launcher_monochrome.xml`
- `apps/tracer_android/app/src/main/res/drawable/ic_launcher_monochrome_symbol.xml`
- `apps/tracer_android/app/src/main/res/mipmap-anydpi-v26/ic_launcher.xml`
- `apps/tracer_android/app/src/main/res/mipmap-anydpi-v26/ic_launcher_round.xml`

## 核心原则

1. SVG 是设计参考源，不是 Android 直接运行时资源
2. Android launcher 最终使用标准 adaptive icon 分层，不再把背景塞进 foreground
3. 前景只承载品牌符号：
   - 箭头
   - 左侧两段轨迹
4. 背景单独承载 `Indigo Mist` full-bleed 浅品牌色背景层
5. 单色图标与彩色前景保持相同 safe zone

## 当前分层策略

### Background

- 资源：`ic_launcher_background.xml`
- 职责：只负责 `Indigo Mist` full-bleed 浅品牌色背景层
- 当前做法：铺满背景层，最终外轮廓由 launcher mask 决定

### Foreground

- 资源：`ic_launcher_foreground.xml`
- 职责：只负责前景 safe zone
- 当前做法：使用 inset 包装 `ic_launcher_foreground_symbol.xml`

- 资源：`ic_launcher_foreground_symbol.xml`
- 职责：只负责彩色箭头与轨迹符号

### Monochrome

- 资源：`ic_launcher_monochrome.xml`
- 职责：只负责单色 safe zone
- 当前做法：使用 inset 包装 `ic_launcher_monochrome_symbol.xml`

- 资源：`ic_launcher_monochrome_symbol.xml`
- 职责：只负责单色箭头与轨迹符号

## 标准应用流程

1. 先修改品牌母版或 Android 平台参考稿
2. 再同步 Android 资源：
   - `ic_launcher_background.xml`
   - `ic_launcher_foreground_symbol.xml`
   - `ic_launcher_monochrome_symbol.xml`
3. 如需调整适配尺寸，优先改 inset，而不是继续在前景内部塞背景
4. 保持 `ic_launcher.xml` 与 `ic_launcher_round.xml` 只做资源引用，不塞平台逻辑

## 为什么这样做

因为不同 Android launcher，特别是 Pixel 与小米等国产桌面，会对 adaptive icon 做不同强度的缩放和遮罩处理。

如果把背景和符号都塞进 foreground：

- launcher 会把整个前景层当成“大图块”
- 结果更容易出现图标看起来过大、贴边、边距不稳定

拆成标准 fg/bg 两层后：

- 白底和符号各自拥有独立层级
- safe zone 可以明确控制
- 更符合 Android 官方 adaptive icon 设计模型
- 能避免某些 launcher 将透明外圈或内嵌白底误表现为额外黑边

## 验证方式

在仓库根目录执行：

```powershell
python scripts/run.py build --app tracer_android --profile android_edit
```

需要确认：

- 构建成功
- 安装后桌面图标边距稳定
- Pixel 与小米等 launcher 下不再出现明显贴边
- 单色主题图标保持可读

## 维护约定

- Android 图标不要再回到“foreground 内部自带背景”的旧做法
- 优先通过：
  - `background` 层
  - `foreground` inset
  - `monochrome` inset
  来调整观感
- 当品牌母版或 Android 平台参考稿改版时，必须同步更新 Android 图标资源

## 相关文档

- `design/branding/README.md`
- `design/branding/platform/android/README.md`
- `apps/tracer_android/icon_generation.md`
