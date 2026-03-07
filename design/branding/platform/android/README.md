# Android Icon Source Mapping

Android 图标使用 `design/branding/master/time_tracer_brand_master_symbol.svg` 作为品牌母版参考。

## 当前默认视觉参考

- `design/branding/exports/bg_golden_vertical_padding_shadow.svg`
- `design/branding/platform/android/time_tracer_android_launcher_reference.svg`

## 说明

- Android 最终资源不是直接使用 SVG
- Android 端以无背景品牌母版为几何与语义基准
- 白底、安全区和阴影属于 Android 平台适配内容
- 当前平台适配稿对前景整体做了更保守的安全区收缩
- 目的是同时兼容 Pixel 与小米等更容易放大图标的桌面
- 当前再结合 `bg_golden_vertical_padding_shadow.svg` 做视觉补偿
- 最终落地到：
  - `apps/tracer_android/app/src/main/res/drawable/ic_launcher_foreground.xml`
  - `apps/tracer_android/app/src/main/res/drawable/ic_launcher_monochrome.xml`

## 相关文档

- `design/branding/android-icon-svg-application.md`
