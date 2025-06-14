# -*- coding: utf-8 -*-
import sqlite3
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.cm as cm
import numpy as np
import argparse
import sys
from typing import Dict, List

def get_ultimate_parent(project_path: str, parent_map: Dict[str, str]) -> str:
    """
    递归查找一个项目的最终父项目。

    Args:
        project_path (str): 当前项目的路径。
        parent_map (Dict[str, str]): 一个包含子项目到父项目映射的字典。

    Returns:
        str: 最终的父项目名称。
    """
    current_project = project_path
    # 循环向上查找，直到找不到父项目或父项目是其自身
    # 使用集合来检测循环引用，防止无限循环
    visited = {current_project}
    while current_project in parent_map and parent_map[current_project] != current_project:
        current_project = parent_map[current_project]
        if current_project in visited:
            # 检测到循环，终止并返回当前项目
            print(f"警告: 在项目 '{project_path}' 的层级中检测到循环。在 '{current_project}' 处中断。", file=sys.stderr)
            break
        visited.add(current_project)
    return current_project

def generate_report(date_str: str, db_path: str):
    """
    连接数据库，处理数据，并生成柱状图报告。

    Args:
        date_str (str): 日期字符串，格式可以是 'YYYY', 'YYYYMM', 或 'YYYYMMDD'。
        db_path (str): SQLite 数据库文件的路径。
    """
    print(f"正在为日期范围 '{date_str}' 生成报告...")

    # 设置matplotlib以支持中文显示
    # 请确保您的系统已安装此字体，或者替换为您可用的中文字体
    try:
        plt.rcParams['font.sans-serif'] = ['SimHei']  # 用于正常显示中文标签
        plt.rcParams['axes.unicode_minus'] = False  # 用于正常显示负号
    except Exception as e:
        print(f"警告: 设置中文字体失败。图表中的中文可能显示为方框。错误: {e}", file=sys.stderr)
        print("建议安装 'SimHei' 字体或在代码中指定一个您系统中可用的中文字体。", file=sys.stderr)

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

        # 2. 数据处理：找到每个记录的最终父项目
        print("正在计算每个项目的父项目...")
        df_records['parent_project'] = df_records['project_path'].apply(
            lambda p: get_ultimate_parent(p, parent_map)
        )

        # 3. 聚合数据：按父项目分组并计算总时长
        print("正在聚合项目时间...")
        project_durations = df_records.groupby('parent_project')['duration'].sum().reset_index()
        
        # 筛选指定的父项目
        allowed_parents = [
            'WORK', 'BLOG', 'STUDY', 'RECREATION', 'MEAL', 'REST', 'HEALTH', 
            'EXERCISE', 'ROUTINE', 'INSOMNIA', 'WASTAGE', 'CORE'
        ]
        project_durations = project_durations[project_durations['parent_project'].isin(allowed_parents)]

        # 如果筛选后没有数据，则不生成图表
        if project_durations.empty:
            print(f"在指定日期范围 '{date_str}' 内，未找到任何预设父项目 ({', '.join(allowed_parents)}) 的活动记录。")
            return

        # 将时长从秒转换为小时并按时长降序排序
        project_durations['duration_hours'] = project_durations['duration'] / 3600
        project_durations = project_durations.sort_values(by='duration_hours', ascending=False)
        
        # 计算所有项目总时长，用于计算百分比
        total_hours = project_durations['duration_hours'].sum()

        # 4. 生成图表
        print("正在生成图表...")
        plt.figure(figsize=(14, 8))
        
        # 根据时长从大到小生成类似GitHub热力图的渐变颜色
        # 使用 'Greens' 色图，时间长的颜色深，时间短的颜色浅
        colors = plt.get_cmap('Greens')(np.linspace(0.9, 0.3, len(project_durations)))

        bars = plt.bar(project_durations['parent_project'], project_durations['duration_hours'], color=colors)

        plt.xlabel('父项目', fontsize=12)
        plt.ylabel('总花费时间 (小时)', fontsize=12)
        
        # 创建一个描述性的标题
        if len(date_str) == 4:
            title_date = f"{date_str}年"
        elif len(date_str) == 6:
            title_date = f"{date_str[:4]}年{date_str[4:]}月"
        else:
            title_date = f"{date_str[:4]}年{date_str[4:6]}月{date_str[6:]}日"
        
        plt.title(f'{title_date} 各父项目时间花费分析', fontsize=16, fontweight='bold')
        
        # 将横坐标标签设置为正常显示（不旋转）
        plt.xticks(rotation=0, ha='center')
        plt.grid(axis='y', linestyle='--', alpha=0.7)

        # 在每个柱子上方显示数值，并带上单位'h'和百分比
        for bar in bars:
            yval = bar.get_height()
            if total_hours > 0:
                percentage = (yval / total_hours) * 100
                label = f'{yval:.2f}h ({percentage:.1f}%)'
            else:
                label = f'{yval:.2f}h' # 避免除以零

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
- 2025 (例如: 2025)
- 202502 (例如: 202502)
- 20250201 (例如: 20250201)"""
    )
    parser.add_argument(
        "--db",
        type=str,
        default="time_data.db",
        help="SQLite数据库文件的路径 (默认为: time_data.db)"
    )

    args = parser.parse_args()

    date_str = args.date
    if not date_str.isdigit() or len(date_str) not in [4, 6, 8]:
        print("错误: 日期格式无效。请输入YYYY, YYYYMM, 或 YYYYMMDD 格式的数字。", file=sys.stderr)
        parser.print_help()
        sys.exit(1)

    generate_report(date_str, args.db)

if __name__ == '__main__':
    main()
