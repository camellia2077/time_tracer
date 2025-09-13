```
graph_generator/
├── configs/
│   ├── heatmap_colors.toml
│   └── timeline_colors.toml
│
├── core/
│   ├── __init__.py
│   ├── app.py                  # 核心：应用逻辑与流程编排
│   └── config.py               # 核心：统一的配置加载
│
├── data/
│   ├── __init__.py
│   ├── db_access.py            # 数据访问层 (DAL)
│   └── day_analyzer.py         # 数据处理与分析
│
├── rendering/
│   ├── __init__.py
│   ├── plotters.py             # 基于 Matplotlib 的绘图器
│   └── heatmap_renderer.py     # 专职渲染 HTML/SVG 热力图
│
├── strategies/
│   ├── __init__.py
│   └── heatmap_strategies.py   # 热力图的“业务逻辑”（如何上色、如何提示）
│
├── ui/
│   ├── __init__.py
│   └── console.py              # 负责所有命令行交互（CLI + 交互菜单）
│
├── main_cli.py                 # 入口：命令行模式
└── main_app.py                 # 入口：交互式菜单模式
```