import sys
import sqlite3
import os
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from datetime import datetime, timedelta

# --- Matplotlib 字体配置 ---
# 为了正确显示中文标签（如果您的活动类别是中文），我们在此设置字体。
# 您需要确保您的系统上安装了所选字体。
# 常见中文字体:
# - Windows: 'SimHei', 'Microsoft YaHei'
# - macOS: 'PingFang SC', 'STHeiti'
# - Linux: 'WenQuanYi Micro Hei', 'Noto Sans CJK SC'
plt.rcParams['font.family'] = 'sans-serif'
plt.rcParams['font.sans-serif'] = ['SimHei']
# 解决保存图像时负号'-'显示为方块的问题
plt.rcParams['axes.unicode_minus'] = False


# --- 配置 ---

# 数据库文件名
DB_FILE = 'time_data.db'

# 父项目与其颜色的映射
COLOR_MAP = {
    'study': 'green',
    'sleep': 'grey',
    'recreation': 'red',
    'break': 'red',
    'exercise': 'lightblue',
    'meal': 'lightblue',
    # 您可以添加更多类别
    'code': 'orange',
    'work': 'purple',
    # 为未指定颜色的类别提供一个默认颜色
    'default': 'silver'
}

# --- 核心功能 ---

def get_daily_activities(db_path, target_date):
    """
    从数据库查询指定日期的所有活动，并找到每个活动的顶级父项目。
    （此函数与之前版本相同，无需修改）
    """
    if not os.path.exists(db_path):
        print(f"错误: 数据库文件 '{db_path}' 不存在。")
        return []

    query = """
    WITH RECURSIVE project_hierarchy(child, parent, top_level_parent) AS (
        SELECT child, parent, child FROM parent_child WHERE child = parent
        UNION ALL
        SELECT pc.child, pc.parent, ph.top_level_parent FROM parent_child pc JOIN project_hierarchy ph ON pc.parent = ph.child WHERE pc.child != pc.parent
    )
    SELECT
        tr.start,
        tr.duration,
        ph.top_level_parent
    FROM
        time_records tr
    JOIN
        project_hierarchy ph ON tr.project_path = ph.child
    WHERE
        tr.date = ?
    ORDER BY
        tr.start;
    """

    records = []
    try:
        with sqlite3.connect(db_path) as conn:
            cursor = conn.cursor()
            cursor.execute(query, (target_date,))
            records = cursor.fetchall()
    except sqlite3.Error as e:
        print(f"数据库查询错误: {e}")
        return []

    return records

def time_str_to_minutes(time_str):
    """将 "HH:MM" 格式的时间字符串转换为从午夜开始的分钟数。"""
    try:
        hours, minutes = map(int, time_str.split(':'))
        return hours * 60 + minutes
    except (ValueError, IndexError):
        return 0

def create_timeline_plot(date_str, activities):
    """
    使用matplotlib创建并显示一个水平时间线图表 (Gantt-like)。

    Args:
        date_str (str): 用于图表标题的日期字符串。
        activities (list): 从数据库获取的活动列表。
    """
    if not activities:
        print(f"日期 {date_str} 没有找到任何活动记录。")
        return

    fig, ax = plt.subplots(figsize=(15, 8))

    # 获取所有独特的父类别，并按字母顺序排序
    parent_categories = sorted(list(set(act[2] for act in activities)))

    # 为每个类别分配一个Y轴位置
    y_positions = {category: i for i, category in enumerate(parent_categories)}

    # 绘制每个活动条
    for start_str, duration_sec, parent in activities:
        start_minutes = time_str_to_minutes(start_str)
        duration_minutes = duration_sec / 60

        # 获取颜色
        color = COLOR_MAP.get(parent, COLOR_MAP['default'])

        # 使用 broken_barh 绘制水平条
        # 第一个参数是 (start, length) 对的列表
        # 第二个参数是 (y_start, y_height)
        ax.broken_barh(
            [(start_minutes, duration_minutes)],
            (y_positions[parent] - 0.4, 0.8), # y位置和条的高度
            facecolors=color,
            edgecolor='black'
        )

    # --- 美化图表 ---

    # 设置图表标题和坐标轴标签
    ax.set_title(f"每日时间线 - {date_str}", fontsize=16, pad=20)
    ax.set_xlabel("一天中的时间", fontsize=12)
    ax.set_ylabel("活动类别", fontsize=12)

    # 设置Y轴（类别轴）
    ax.set_yticks(range(len(parent_categories)))
    ax.set_yticklabels(parent_categories)
    ax.set_ylim(-0.5, len(parent_categories) - 0.5) # 设置Y轴范围以适应所有类别

    # 设置X轴（时间轴）
    ax.set_xlim(0, 24 * 60)
    # 设置X轴的刻度，每2小时一个主刻度
    x_ticks = range(0, 24 * 60 + 1, 120)
    x_tick_labels = [f"{h:02d}:00" for h in range(0, 25, 2)]
    ax.set_xticks(x_ticks)
    ax.set_xticklabels(x_tick_labels, rotation=45, ha='right')

    # 添加垂直网格线，方便对齐时间
    ax.grid(axis='x', linestyle='--', alpha=0.7)

    # 反转Y轴，使类别列表看起来更直观（A, B, C...从上到下）
    ax.invert_yaxis()

    # 创建一个智能图例
    # 在这个图表中，Y轴标签已经起到了图例的作用，所以我们不再需要独立的图例。
    # 如果您仍然想要它，可以取消下面的注释
    # legend_patches = [mpatches.Patch(color=COLOR_MAP.get(cat, 'silver'), label=cat) for cat in parent_categories]
    # ax.legend(handles=legend_patches, bbox_to_anchor=(1.02, 1), loc='upper left')

    # 调整布局以防止标签重叠
    plt.tight_layout()

    # 显示图表
    plt.show()

def main():
    """
    主函数，处理命令行参数并启动程序。
    (此函数与之前版本相同，无需修改)
    """
    if len(sys.argv) != 2:
        print("用法: python plot_horizontal_timeline.py YYYYMMDD")
        print("示例: python plot_horizontal_timeline.py 20250301")
        sys.exit(1)

    target_date = sys.argv[1]

    if not (target_date.isdigit() and len(target_date) == 8):
        print("错误: 日期格式不正确，请输入8位数字，例如 20250301。")
        sys.exit(1)

    print(f"正在为日期 {target_date} 生成水平时间线图表...")

    activities = get_daily_activities(DB_FILE, target_date)
    create_timeline_plot(target_date, activities)

if __name__ == "__main__":
    main()