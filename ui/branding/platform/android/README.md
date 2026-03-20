# Android Icon Source Mapping

Android 图标使用 `design/branding/master/time_tracer_brand_master_symbol.svg` 作为品牌母版参考。

## 当前默认平台参考

- `design/branding/platform/android/time_tracer_android_launcher_reference.svg`

## Android 最终资源分层

Android launcher 采用标准 adaptive icon 分层：

- `apps/android/app/src/main/res/drawable/ic_launcher_background.xml`
  - `Indigo Mist` full-bleed 浅品牌色背景层
- `apps/android/app/src/main/res/drawable/ic_launcher_foreground.xml`
  - 前景 inset 包装层
- `apps/android/app/src/main/res/drawable/ic_launcher_foreground_symbol.xml`
  - 箭头与轨迹符号
- `apps/android/app/src/main/res/drawable/ic_launcher_monochrome.xml`
  - 单色 inset 包装层
- `apps/android/app/src/main/res/drawable/ic_launcher_monochrome_symbol.xml`
  - 单色箭头与轨迹符号

## 说明

- Android 最终资源不是直接使用 SVG
- Android 端以无背景品牌母版为识别核心
- 背景、safe zone 与 adaptive icon 分层属于 Android 平台适配内容
- 背景层由应用提供 `Indigo Mist` full-bleed 浅品牌色底色
- 最终外轮廓圆角交给 launcher mask 决定
- 当前实现不再采用“前景里同时塞背景 + 阴影近似”的兼容方案
- 当前目标是让图标在 Pixel 与小米等更容易放大图标的桌面上都保持稳定边距

## 相关文档

- `design/branding/android-icon-svg-application.md`
