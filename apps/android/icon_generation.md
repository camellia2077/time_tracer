# Android Icon 生成与替换流程

本文用于说明如何在本项目中更新 Android launcher icon，并与 `design/branding/**` 下的设计源保持一致。

> 目录约定说明（2026-03-07）
> - 品牌母版统一放在 `design/branding/master/**`
> - Android 平台适配稿统一放在 `design/branding/platform/android/**`
> - Android 最终图标遵循标准 adaptive icon 的 `background / foreground / monochrome` 分层

## 1. 当前设计源

当前 Android 图标采用以下两级设计源：

- 品牌母版：`design/branding/master/time_tracer_brand_master_symbol.svg`
- Android 平台参考稿：`design/branding/platform/android/time_tracer_android_launcher_reference.svg`

## 2. 当前 Android 图标资源

Android launcher 相关资源：

- `apps/android/app/src/main/res/drawable/ic_launcher_background.xml`
- `apps/android/app/src/main/res/drawable/ic_launcher_foreground.xml`
- `apps/android/app/src/main/res/drawable/ic_launcher_foreground_symbol.xml`
- `apps/android/app/src/main/res/drawable/ic_launcher_monochrome.xml`
- `apps/android/app/src/main/res/drawable/ic_launcher_monochrome_symbol.xml`
- `apps/android/app/src/main/res/mipmap-anydpi-v26/ic_launcher.xml`
- `apps/android/app/src/main/res/mipmap-anydpi-v26/ic_launcher_round.xml`

## 3. 当前默认方案

推荐默认组合如下：

- Android 背景：`ic_launcher_background.xml` 负责 `Indigo Mist` full-bleed 浅品牌色背景层
- Android 前景：`ic_launcher_foreground.xml` 负责 safe zone inset
- Android 彩色符号：`ic_launcher_foreground_symbol.xml` 负责箭头与轨迹
- Android 单色图标：`ic_launcher_monochrome.xml` + `ic_launcher_monochrome_symbol.xml`

## 4. 标准操作步骤

1. 确认品牌母版与 Android 平台参考稿
2. 若修改底板，编辑：
   - `ic_launcher_background.xml`
3. 若修改彩色符号，编辑：
   - `ic_launcher_foreground_symbol.xml`
4. 若修改单色符号，编辑：
   - `ic_launcher_monochrome_symbol.xml`
5. 若需要调整 Android 桌面观感，优先修改：
   - `ic_launcher_foreground.xml` 的 inset
   - `ic_launcher_monochrome.xml` 的 inset
   - 如确有必要，再调整 `ic_launcher_background.xml`
6. 保持以下文件只做引用，不塞实现逻辑：
   - `ic_launcher.xml`
   - `ic_launcher_round.xml`

## 5. 当前实现原则

- 不再把白色底板塞进 foreground
- 不再依赖阴影近似来决定桌面观感
- 不再使用透明背景 + 前景内白底的兼容做法
- 不再在应用资源里硬编码最终圆角外轮廓
- 最终外轮廓交给 launcher mask 决定
- Android 桌面兼容问题优先通过标准 fg/bg 两层与 safe zone 解决

## 6. 验证方式

```powershell
python tools/run.py build --app tracer_android --profile android_edit
```

建议验证：

- 安装后看桌面图标，而不是只看 APK 缩略图
- 同时观察 Pixel 风格桌面与小米等国产桌面
- 确认图标没有贴边、没有显得过大

## 7. 与其他端的关系

- Windows 图标流程见：`apps/tracer_cli/windows/icon_generation.md`
- Android 与 Windows 共用同一套品牌母版：
  - `design/branding/master/time_tracer_brand_master_symbol.svg`
- 两端最终资源不同：
  - Windows：构建期转 `.ico`
  - Android：手动维护 adaptive icon 资源分层
