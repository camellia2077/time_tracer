# Time Tracer Branding Assets

本目录是 Time Tracer 品牌设计参考稿的长期归档位置。

## 目录结构

- `exports/`
  - 当前可直接作为平台图标设计源使用的稳定 SVG 导出稿
- `reference/`
  - 设计哲学、视觉说明、选型依据
- `explorations/`
  - 历史探索稿、实验稿、版本迭代稿

## 使用约定

1. 这里存放的是设计参考资产，不是 runtime asset。
2. 程序运行时配置的 canonical source 仍然是：
   - `assets/tracer_core/config`
3. Android / Windows 如需引用 SVG 作为设计源或导出源，
   应优先从 `design/branding/exports/**` 选择。
4. 不再在本目录树之外新增长期设计资产。
5. 旧的共享设计目录已退役删除；历史设计资产统一归档在本目录树内。
6. 交互式设计演示文件位于：
   - `design/branding/explorations/demo/demo.html`

## 当前推荐文件

- 默认圆角白底：
  - `design/branding/exports/sharp_rounded_white_golden.svg`
- 透明底备选：
  - `design/branding/exports/bg_golden_vertical_padding_transparent.svg`
