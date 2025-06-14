import sys
import sqlite3
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from matplotlib.font_manager import FontProperties
import os

# --- 配置区 ---
# 脚本将读取同目录下的 'time_data.db' 文件
DB_PATH = 'time_data.db' 
# 指定一个支持中文的字体，例如 'SimHei', 'Microsoft YaHei' 等
# 如果在非 Windows 系统上运行，请确保系统已安装此字体
# 如果留空，Matplotlib 会使用默认字体，可能导致中文显示为方框
CHINESE_FONT_PATH = 'C:/Windows/Fonts/SimHei.ttc' # 例如 'C:/Windows/Fonts/msyh.ttc'

# 定义父项目与颜色的映射关系
# 修改：根据要求，将 SLEEP 设置为灰色，其余项目设置为淡灰色
COLOR_MAP = {
    'STUDY': 'lightgrey',
    'REST': 'lightgrey',
    'MEAL': 'lightgrey',
    'RECREATION': 'lightgrey',
    'BREAK': 'lightgrey',
    'SLEEP': 'grey'  # 将 SLEEP 的颜色设置为灰色
}
DEFAULT_COLOR = 'lightgrey' # 将默认颜色也设置为淡灰色

def get_top_parent(child: str, parent_map: dict) -> str:
    """
    递归查找一个项目的顶级父项目。
    
    Args:
        child: 子项目的路径。
        parent_map: 包含所有父子关系的字典。

    Returns:
        该项目的顶级父项目名称。
    """
    parent = parent_map.get(child, child)
    # 顶级项目的父项目通常是其自身的大写形式或不存在于映射的键中
    while child != parent and child in parent_map:
        child = parent
        parent = parent_map.get(child, child)
    return parent

def create_activity_plot(date_str: str, db_path: str):
    """
    从数据库读取指定日期的数据，并生成活动图。

    Args:
        date_str: 日期字符串，格式为 "YYYYMMDD"。
        db_path: SQLite 数据库文件的路径。
    """
    print(f"正在为日期 {date_str} 生成图表...")

    if not os.path.exists(db_path):
        print(f"错误：数据库文件 '{db_path}' 未找到。")
        print("请检查 DB_PATH变量或确保文件与脚本在同一目录中。")
        return

    try:
        # 使用 'with' 语句确保数据库连接自动关闭
        with sqlite3.connect(db_path) as conn:
            # 1. 获取父子关系映射
            parent_child_df = pd.read_sql_query("SELECT child, parent FROM parent_child", conn)
            parent_map = pd.Series(parent_child_df.parent.values, index=parent_child_df.child).to_dict()

            # 2. 获取指定日期的活动记录
            query = "SELECT start, end, project_path, duration FROM time_records WHERE date = ? ORDER BY start"
            df = pd.read_sql_query(query, conn, params=(date_str,))

        if df.empty:
            print(f"日期 {date_str} 没有找到任何活动记录。")
            return

        # 3. 数据处理
        # 找到每个活动的顶级父项目
        df['parent_project'] = df['project_path'].apply(lambda x: get_top_parent(x, parent_map))

        # 计算每个父项目的当日总时长（单位：秒）
        parent_total_durations = df.groupby('parent_project')['duration'].sum().reset_index()
        parent_total_durations.rename(columns={'duration': 'parent_total_duration'}, inplace=True)

        # 将父项目总时长合并回主 DataFrame
        df = pd.merge(df, parent_total_durations, on='parent_project')

        # 将时间字符串转换为 datetime 对象以便绘图
        df['start_dt'] = pd.to_datetime(date_str + ' ' + df['start'], format='%Y%m%d %H:%M')
        
        # 将持续时间（秒）转换为 Timedelta 对象
        df['duration_td'] = pd.to_timedelta(df['duration'], unit='s')

    except sqlite3.Error as e:
        print(f"数据库错误: {e}")
        return
    except Exception as e:
        print(f"处理数据时发生错误: {e}")
        return

    # 4. 开始绘图
    fig, ax = plt.subplots(figsize=(8, 15))

    # 设置中文字体
    try:
        font_prop = FontProperties(fname=CHINESE_FONT_PATH) if CHINESE_FONT_PATH else FontProperties()
    except Exception as e:
        print(f"警告：加载字体时出错 '{CHINESE_FONT_PATH}'。将使用默认字体。错误：{e}")
        font_prop = FontProperties()

    # Y 轴 (时间)
    ax.set_ylabel('时间', fontproperties=font_prop, fontsize=12)
    min_time = pd.to_datetime(f'{date_str} 00:00')
    max_time = pd.to_datetime(f'{date_str} 23:59:59')
    ax.set_ylim(min_time, max_time)
    ax.yaxis.set_major_formatter(mdates.DateFormatter('%H:%M'))
    ax.yaxis.set_major_locator(mdates.HourLocator(interval=1))
    ax.invert_yaxis() # 时间从上到下递增

    # X 轴 (隐藏)
    ax.get_xaxis().set_visible(False)
    ax.set_xlim(0, 5)

    # 绘制条形和标签
    labeled_parents = set()
    for _, row in df.iterrows():
        parent_name = row['parent_project']
        start_time = row['start_dt']
        duration = row['duration_td']
        color = COLOR_MAP.get(parent_name.upper(), DEFAULT_COLOR)
        
        # 使用 barh 绘制横向条形图
        # y: 条形的起始垂直位置
        # height: 条形的垂直厚度 (即活动持续时间)
        # width: 条形的横向长度 (固定值)
        # left: 条形的起始水平位置
        ax.barh(y=start_time, height=duration, width=2.0, left=0.5,
                color=color, edgecolor='black', linewidth=0.7, align='edge')

        # 添加标签，为每个父项目只在第一次出现时标注，以防重叠
        if parent_name not in labeled_parents:
            # 将父项目总秒数格式化为小时
            total_hours = row['parent_total_duration'] / 3600.0
            label_text = f"{parent_name} ({total_hours:.1f}h)"
            
            # 标签的垂直位置在当前条形的中间
            label_y_pos = start_time + duration / 2
            
            # 确保标签在绘图区域内
            if label_y_pos > max_time: label_y_pos = max_time
            if label_y_pos < min_time: label_y_pos = min_time

            ax.text(2.7, label_y_pos, label_text, 
                    verticalalignment='center', fontsize=10, fontproperties=font_prop)
            labeled_parents.add(parent_name)

    # 设置图表标题和布局
    ax.set_title(f'{date_str} 活动记录图', fontproperties=font_prop, fontsize=16, pad=20)
    plt.tight_layout()

    # 保存图表
    output_filename = f'activity_plot_{date_str}.png'
    try:
        plt.savefig(output_filename, dpi=300)
        print(f"图表已成功保存为 '{output_filename}'")
    except Exception as e:
        print(f"保存文件时出错: {e}")

    # plt.show() # 如果需要直接显示图表，取消此行注释

def main():
    """主函数，用于解析命令行参数并调用绘图函数。"""
    if len(sys.argv) != 2:
        print("用法: python create_plot.py YYYYMMDD")
        sys.exit(1)
        
    date_to_plot = sys.argv[1]
    
    if len(date_to_plot) != 8 or not date_to_plot.isdigit():
        print("错误：日期格式不正确。请使用 YYYYMMDD 格式，例如 20250101。")
        sys.exit(1)
        
    create_activity_plot(date_to_plot, DB_PATH)

if __name__ == '__main__':
    main()
