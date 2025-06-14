# -*- coding: utf-8 -*-
import sqlite3
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors # 导入 mcolors 用于更灵活的颜色处理
import numpy as np
import argparse
import sys
import json # 导入 json 用于读取配置文件
from typing import Dict, List, Optional

def load_config(config_path: str = 'heatmap_colors_config.json') -> dict:
    """
    从 JSON 文件加载颜色配置。
    """
    try:
        with open(config_path, 'r', encoding='utf-8') as f:
            return json.load(f)
    except FileNotFoundError:
        print(f"错误: 配置文件 '{config_path}' 未找到。", file=sys.stderr)
        sys.exit(1)
    except json.JSONDecodeError:
        print(f"错误: 无法解析 '{config_path}' 中的 JSON。请检查文件格式。", file=sys.stderr)
        sys.exit(1)

def get_ultimate_parent(project_path: str, parent_map: Dict[str, str]) -> str:
    """
    递归查找一个项目的最终父项目。
    """
    current_project = project_path
    visited = {current_project}
    while current_project in parent_map and parent_map[current_project] != current_project:
        current_project = parent_map[current_project]
        if current_project in visited:
            print(f"警告: 在项目 '{project_path}' 的层级中检测到循环。在 '{current_project}' 处中断。", file=sys.stderr)
            break
        visited.add(current_project)
    return current_project

def generate_report(date_str: str, db_path: str, config: dict, palette_name: Optional[str] = None):
    """
    连接数据库，处理数据，并生成柱状图报告。
    """
    print(f"正在为日期范围 '{date_str}' 生成报告...")

    try:
        plt.rcParams['font.sans-serif'] = ['SimHei']
        plt.rcParams['axes.unicode_minus'] = False
    except Exception as e:
        print(f"警告: 设置中文字体失败。图表中的中文可能显示为方框。错误: {e}", file=sys.stderr)

    conn = None
    try:
        # 1. 连接数据库并获取数据
        conn = sqlite3.connect(db_path)
        
        query_records = "SELECT project_path, duration FROM time_records WHERE date LIKE ?"
        df_records = pd.read_sql_query(query_records, conn, params=(date_str + '%',))

        if df_records.empty:
            print(f"在数据库 '{db_path}' 中找不到日期范围 '{date_str}' 的任何记录。")
            return

        df_parents = pd.read_sql_query("SELECT child, parent FROM parent_child", conn)
        parent_map = pd.Series(df_parents.parent.values, index=df_parents.child).to_dict()

        # 2. 数据处理
        print("正在计算每个项目的父项目...")
        df_records['parent_project'] = df_records['project_path'].apply(
            lambda p: get_ultimate_parent(p, parent_map)
        )

        # 3. 聚合数据
        print("正在聚合项目时间...")
        project_durations = df_records.groupby('parent_project')['duration'].sum().reset_index()
        
        allowed_parents = [
            'WORK', 'BLOG', 'STUDY', 'RECREATION', 'MEAL', 'REST', 'HEALTH', 
            'EXERCISE', 'ROUTINE', 'INSOMNIA', 'WASTAGE', 'CORE'
        ]
        project_durations = project_durations[project_durations['parent_project'].isin(allowed_parents)]

        if project_durations.empty:
            print(f"在指定日期范围 '{date_str}' 内，未找到任何预设父项目的活动记录。")
            return

        project_durations['duration_hours'] = project_durations['duration'] / 3600
        project_durations = project_durations.sort_values(by='duration_hours', ascending=False)
        
        total_hours = project_durations['duration_hours'].sum()

        # 4. 生成图表
        print("正在生成图表...")
        plt.figure(figsize=(14, 8))
        
        # --- 新的颜色生成逻辑 ---
        # 从配置中选择调色板
        color_palettes = config['COLOR_PALETTES']
        chosen_palette_name = palette_name
        
        if chosen_palette_name and chosen_palette_name not in color_palettes:
            print(f"警告: 在配置中未找到调色板 '{chosen_palette_name}'。将使用默认调色板。")
            chosen_palette_name = None

        if not chosen_palette_name:
            chosen_palette_name = config.get('DEFAULT_COLOR_PALETTE_NAME', 'GITHUB_GREEN_LIGHT')
        
        print(f"正在使用调色板: '{chosen_palette_name}'")
        selected_palette = color_palettes[chosen_palette_name]

        num_bars = len(project_durations)
        
        # 使用调色板中除第一个（通常是最浅的）之外的颜色来创建渐变
        # 这确保我们从有意义的颜色开始渐变
        gradient_colors = selected_palette[1:] if len(selected_palette) > 1 else selected_palette
        
        # 将颜色列表反转，使其从深到浅
        gradient_colors.reverse()
        
        # 创建一个线性分段的颜色映射
        cmap = mcolors.LinearSegmentedColormap.from_list('custom_gradient', gradient_colors)
        
        # 根据柱子的数量从颜色映射中生成颜色
        # np.linspace(0, 1, num_bars) 会生成一个从0到1的等差数列
        # cmap会根据这些值从我们定义的渐变中取色
        colors = cmap(np.linspace(0, 1, num_bars))
        # --- 颜色逻辑结束 ---

        bars = plt.bar(project_durations['parent_project'], project_durations['duration_hours'], color=colors)

        plt.xlabel('父项目', fontsize=12)
        plt.ylabel('总花费时间 (小时)', fontsize=12)
        
        if len(date_str) == 4:
            title_date = f"{date_str}年"
        elif len(date_str) == 6:
            title_date = f"{date_str[:4]}年{date_str[4:]}月"
        else:
            title_date = f"{date_str[:4]}年{date_str[4:6]}月{date_str[6:]}日"
        
        plt.title(f'{title_date} 各父项目时间花费分析', fontsize=16, fontweight='bold')
        
        plt.xticks(rotation=0, ha='center')
        plt.grid(axis='y', linestyle='--', alpha=0.7)

        for bar in bars:
            yval = bar.get_height()
            percentage = (yval / total_hours) * 100 if total_hours > 0 else 0
            label = f'{yval:.2f}h\n({percentage:.1f}%)'
            plt.text(bar.get_x() + bar.get_width()/2.0, yval, label, va='bottom', ha='center', fontsize=10)

        plt.tight_layout()

        # 5. 保存图表
        output_filename = f'report_{date_str}.png'
        plt.savefig(output_filename)
        print(f"报告生成成功！图表已保存为 '{output_filename}'")

    except sqlite3.Error as e:
        print(f"数据库错误: {e}", file=sys.stderr)
    except FileNotFoundError:
        print(f"错误: 数据库文件 '{db_path}' 未找到。", file=sys.stderr)
    except Exception as e:
        print(f"发生未知错误: {e}", file=sys.stderr)
    finally:
        if conn:
            conn.close()

def main():
    """
    主函数，用于解析命令行参数并启动报告生成过程。
    """
    parser = argparse.ArgumentParser(
        description="根据SQLite数据库中的时间记录生成父项目花费时间的柱状图。",
        formatter_class=argparse.RawTextHelpFormatter
    )
    parser.add_argument(
        "date",
        type=str,
        help="""要查询的日期。格式可以是：
- YYYY (例如: 2025)
- YYYYMM (例如: 202502)
- YYYYMMDD (例如: 20250201)"""
    )
    parser.add_argument(
        "--db",
        type=str,
        default="time_data.db",
        help="SQLite数据库文件的路径 (默认为: time_data.db)"
    )
    # 添加 --palette 参数
    parser.add_argument(
        "--palette",
        type=str,
        help="要使用的调色板名称 (需在 heatmap_colors_config.json 中定义)。"
    )

    args = parser.parse_args()

    date_str = args.date
    if not date_str.isdigit() or len(date_str) not in [4, 6, 8]:
        print("错误: 日期格式无效。请输入YYYY, YYYYMM, 或 YYYYMMDD 格式的数字。", file=sys.stderr)
        parser.print_help()
        sys.exit(1)
        
    # 加载颜色配置文件
    config = load_config()

    generate_report(date_str, args.db, config, palette_name=args.palette)

if __name__ == '__main__':
    main()
