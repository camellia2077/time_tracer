# Time Tracer Branding Assets

`design/branding` 用于存放 Time Tracer 的长期品牌设计参考资源。

这里的文件主要服务于：

- 品牌母版
- 平台图标设计源
- 设计参考与探索稿

这里不是运行时资源目录。

## 三层命名结构

`design/branding` 现在明确分为三层：

1. `design/branding/master/`
   - 品牌母版层
   - 保存跨平台共享的核心几何、比例、留白和识别语义
2. `design/branding/platform/`
   - 平台适配层
   - 说明各平台如何从品牌母版落地为最终资源
3. `design/branding/explorations/`
   - 设计探索层
   - 保存历史探索稿、实验稿、候选稿

辅助目录：

- `design/branding/exports/`
  - 当前稳定可用的 SVG 导出
- `design/branding/reference/`
  - 设计理念、视觉说明、选型依据

## 当前品牌母版

当前正式品牌母版为：

- `design/branding/master/time_tracer_brand_master_symbol.svg`

这份母版只保留跨平台共享的品牌核心符号：

- 箭头大小
- 箭头位置
- 左侧两段轨迹条
- 整体节奏与相对关系

以下内容不再写死在品牌母版中，而交给平台适配层决定：

- 白色圆角底
- 安全区
- 阴影
- 平台背景

历史上较早形成的一份带背景组合稿仍保留为参考：

- `design/branding/master/time_tracer_brand_master.svg`

## 平台适配策略

- Android
  - 参考无背景品牌母版几何
  - 默认平台参考：`design/branding/platform/android/time_tracer_android_launcher_reference.svg`
  - 最终落地为 Android adaptive icon 的 `background / foreground / monochrome` 三个资源
- Windows CLI
  - 参考无背景品牌母版几何
  - 默认构建源：`design/branding/exports/bg_indigo_mist_vertical_padding.svg`
  - 最终在构建期转为 `.ico`

平台映射文档：

- `design/branding/platform/android/README.md`
- `design/branding/platform/windows_cli/README.md`

## 使用约定

1. 本目录存放的是设计参考资源，不是应用运行时资源
2. 程序运行时配置的 canonical source 仍然是：
   - `assets/tracer_core/config`
3. 跨平台共享的视觉核心，优先固化到 `design/branding/master/`
4. 平台实现差异，写到 `design/branding/platform/`
5. 稳定可复用的 SVG 导出，保留在 `design/branding/exports/`
6. 新的长期品牌资源，统一收口到 `design/branding/**`
7. 不再在仓库其他位置新增长期共享设计资源目录
8. 交互式设计演示文件位于：
   - `design/branding/explorations/demo/demo.html`

## 平台落地文档

- Android 图标 SVG 应用说明：
  - `design/branding/android-icon-svg-application.md`
- Windows CLI 图标 SVG 应用说明：
  - `design/branding/windows-cli-icon-svg-application.md`

## 相关说明

- Android 当前使用 `vector drawable`，不会直接加载 SVG
- Windows CLI 会在构建期把 SVG 转为 `.ico` 并注入可执行文件
