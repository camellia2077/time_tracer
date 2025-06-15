# -*- coding: utf-8 -*-
"""
每日活动时间线生成器

本脚本通过连接一个SQLite数据库来查询指定日期的时间记录，
并使用Matplotlib生成一个可视化的时间线图表。

功能:
- 默认读取同目录下的 'time_data.db' 数据库文件。
- 从命令行接收日期作为参数。
- 查询数据库中指定日期的所有时间记录。
- Y轴标签仅显示活动的父项目（如 'study', 'code'）。
- 将时间轴固定在 00:00 到 24:00，并裁剪超出此范围的活动。
- 为不同类型的活动（学习、休息、睡眠等）分配特定颜色。
- 支持中文显示。

用法:
python <脚本名>.py <日期>
例如:
python create_timeline.py 20250401
"""
import sqlite3
import sys
import os
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from datetime import datetime, timedelta

def configure_matplotlib_for_chinese():
    """
    配置Matplotlib以正确显示中文字符。
    它会尝试使用系统中常见的几种中文字体。
    """
    try:
        # 优先使用黑体
        plt.rcParams['font.sans-serif'] = ['SimHei'] 
        plt.rcParams['axes.unicode_minus'] = False # 解决负号显示为方块的问题
        print("Matplotlib 已配置为使用 'SimHei' 字体。")
    except:
        try:
            # 如果没有黑体，尝试使用微软雅黑
            plt.rcParams['font.sans-serif'] = ['Microsoft YaHei']
            plt.rcParams['axes.unicode_minus'] = False
            print("Matplotlib 已配置为使用 'Microsoft YaHei' 字体。")
        except:
            print("警告: 未找到 'SimHei' 或 'Microsoft YaHei' 字体。图表中的中文可能无法正常显示。")
            print("请安装中文字体（如 'SimHei'）或修改脚本以使用您系统中的可用字体。")

def get_color_for_project(project_path):
    """
    根据项目路径的关键词返回对应的颜色。
    - 'study': 绿色
    - 'sleep': 深灰色
    - 'recreation': 红色
    - 'rest', 'meal', 'exercise', 'routine': 灰色
    - 其他: 蓝色
    """
    project_lower = project_path.lower()
    if 'study' in project_lower:
        return 'mediumseagreen' # 绿色
    elif 'sleep' in project_lower:
        return '#4F4F4F' # 深灰色
    elif 'recreation' in project_lower:
        return "#FF0404" # 红色
    elif any(keyword in project_lower for keyword in ['rest',  'meal', 'exercise', 'routine']):
        return 'silver' # 灰色
    else:
        return 'cornflowerblue' # 默认蓝色

def fetch_daily_records(db_path, target_date):
    """
    从SQLite数据库中获取指定日期的时间记录。

    Args:
        db_path (str): 数据库文件路径。
        target_date (str): 目标日期，格式为 'YYYYMMDD'。

    Returns:
        list: 包含 (start, end, project_path) 元组的列表。
              如果没有记录或发生错误，则返回空列表。
    """
    if not os.path.exists(db_path):
        print(f"错误: 数据库文件 '{db_path}' 未找到。请确保它与脚本在同一目录下。")
        return []

    records = []
    try:
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()
        
        # 按开始时间排序，确保记录按时间顺序排列
        query = "SELECT start, end, project_path FROM time_records WHERE date = ? ORDER BY start ASC"
        cursor.execute(query, (target_date,))
        records = cursor.fetchall()
        
        conn.close()
        
        if not records:
            print(f"在日期 {target_date} 没有找到任何记录。")
            
        return records
    except sqlite3.Error as e:
        print(f"数据库错误: {e}")
        return []

def plot_timeline(records, target_date):
    """
    使用Matplotlib绘制时间线图表。

    Args:
        records (list): 从数据库获取的记录列表。
        target_date (str): 目标日期，用于图表标题和文件名。
    """
    if not records:
        return

    # 1. 数据处理：将时间字符串转换为datetime对象并裁剪到00:00-24:00窗口
    # -----------------------------------------------------------------
    plot_data = []
    base_date_obj = datetime.strptime(target_date, '%Y%m%d')
    
    # 定义00:00到24:00的裁剪窗口
    window_start = base_date_obj
    window_end = base_date_obj + timedelta(days=1)

    for i, (start_str, end_str, project_path) in enumerate(records):
        try:
            # 解析开始和结束时间
            start_time = datetime.strptime(start_str, '%H:%M').time()
            end_time = datetime.strptime(end_str, '%H:%M').time()
            
            # 组合成完整的datetime对象
            start_dt = base_date_obj.combine(base_date_obj.date(), start_time)
            end_dt = base_date_obj.combine(base_date_obj.date(), end_time)

            # 处理原始的跨天事件
            if end_dt < start_dt:
                end_dt += timedelta(days=1)

            # **新增逻辑：将活动裁剪到00:00-24:00的窗口内**
            plot_start = max(start_dt, window_start)
            plot_end = min(end_dt, window_end)

            # 如果裁剪后时间段有效（时长大于0），则添加到绘图列表
            if plot_start < plot_end:
                # 获取父项目作为标签
                parent_project = project_path.split('_')[0]
                    
                plot_data.append({
                    'label': parent_project,
                    'full_path': project_path, # 保留完整路径用于颜色判断
                    'start': plot_start, # 使用裁剪后的开始时间
                    'end': plot_end,   # 使用裁剪后的结束时间
                    'color': get_color_for_project(project_path)
                })
        except ValueError as e:
            print(f"警告: 跳过格式错误的时间记录: {start_str}-{end_str}. 错误: {e}")
            continue

    if not plot_data:
        print("没有在00:00-24:00时间范围内的有效活动可供绘图。")
        return

    # 2. Matplotlib 绘图设置
    # -----------------------------------------------------------------
    fig, ax = plt.subplots(figsize=(15, 10)) # 增加了图表高度以容纳标签

    # 准备Y轴标签和位置
    # Y轴显示父项目，但每个条目仍然是独立的
    labels = [f"{item['label']} ({item['start'].strftime('%H:%M')}-{item['end'].strftime('%H:%M')})" for item in plot_data]
    y_pos = range(len(plot_data))

    # 3. 绘制水平条形图 (Gantt-like chart)
    # -----------------------------------------------------------------
    for i, data in enumerate(plot_data):
        # matplotlib的日期格式是浮点数，需要转换
        start_num = mdates.date2num(data['start'])
        end_num = mdates.date2num(data['end'])
        duration_num = end_num - start_num
        
        # 使用 broken_barh 绘制条形
        ax.broken_barh([(start_num, duration_num)], (i - 0.4, 0.8), facecolors=data['color'])

    # 4. 格式化图表
    # -----------------------------------------------------------------
    ax.set_yticks(y_pos)
    ax.set_yticklabels(labels)
    ax.invert_yaxis()  # 将第一个活动放在顶部

    # 格式化X轴为时间
    ax.xaxis.set_major_formatter(mdates.DateFormatter('%H:%M'))
    ax.xaxis.set_major_locator(mdates.HourLocator(interval=2)) # 每2小时一个主刻度
    ax.xaxis.set_minor_locator(mdates.HourLocator(interval=1)) # 每1小时一个次刻度

    # **修改逻辑：设置固定的X轴范围**
    ax.set_xlim(window_start, window_end)
    
    # 设置标题和标签
    ax.set_title(f'{target_date[:4]}年{target_date[4:6]}月{target_date[6:]}日 时间线 (00:00 - 24:00)', fontsize=16)
    ax.set_xlabel('时间', fontsize=12)
    ax.set_ylabel('活动', fontsize=12)

    # 添加网格线并美化
    ax.grid(True, which='major', axis='x', linestyle='--', linewidth=0.5)
    fig.autofmt_xdate() # 自动旋转X轴标签以防重叠
    plt.tight_layout()

    # 5. 保存图表
    # -----------------------------------------------------------------
    output_filename = f'timeline_{target_date}.png'
    plt.savefig(output_filename, dpi=150)
    print(f"图表已成功保存为: {output_filename}")
    # plt.show() # 如果想直接显示图表，取消此行注释

def main():
    """
    主执行函数
    """
    # 配置字体
    configure_matplotlib_for_chinese()

    # 检查命令行参数
    if len(sys.argv) != 2:
        print("用法: python create_timeline.py <日期YYYYMMDD>")
        print("例如: python create_timeline.py 20250401")
        sys.exit(1)

    # 默认数据库路径
    db_path = 'time_data.db'
    target_date = sys.argv[1]

    # 校验日期格式
    if len(target_date) != 8 or not target_date.isdigit():
        print("错误: 日期格式必须为 YYYYMMDD (例如: 20250401)")
        sys.exit(1)
    
    # 获取数据并绘图
    records = fetch_daily_records(db_path, target_date)
    if records:
        plot_timeline(records, target_date)

if __name__ == "__main__":
    main()
