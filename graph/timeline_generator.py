# -*- coding: utf-8 -*-
"""
每日活动时间线生成器 (双视图版)

本脚本通过连接一个SQLite数据库来查询指定日期的时间记录，
并使用Matplotlib生成两种可视化的时间线图表：
1. 详细视图：每个活动片段单独占一行。
2. 分组视图：同类活动合并到同一行，并输出时长统计。

功能:
- 默认读取同目录下的 'time_data.db' 数据库文件。
- 从命令行接收日期作为参数。
- 将时间轴固定在 00:00 到 24:00，并裁剪超出此范围的活动。
- 支持中文显示。
- 在分组视图中，为每个时长超过30分钟的独立活动片段标注时长和占比。

用法:
python <脚本名>.py <日期>
例如:
python timeline_generator.py 20250401
"""
import sqlite3
import sys
import os
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import matplotlib.patheffects as path_effects # 导入用于文本效果的模块
from datetime import datetime, timedelta
from collections import defaultdict

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
    - 使用字典映射实现，代替if-elif结构。
    """
    # 颜色映射表：关键词 -> 颜色
    color_map = {
        'study': 'mediumseagreen',      # 绿色
        'sleep': '#4F4F4F',           # 深灰色
        'recreation': '#FF0404',      # 红色
        'rest': 'silver',             # 灰色
        'meal': 'silver',             # 灰色
        'exercise': 'silver',         # 灰色
        'routine': 'silver',          # 灰色
    }
    
    # 默认颜色
    default_color = 'cornflowerblue' # 默认蓝色
    
    project_lower = project_path.lower()

    # 遍历映射表，查找第一个匹配的关键词
    for keyword, color in color_map.items():
        if keyword in project_lower:
            return color
            
    # 如果没有找到任何关键词，返回默认颜色
    return default_color

def fetch_daily_records(db_path, target_date):
    """
    从SQLite数据库中获取指定日期的时间记录。
    """
    if not os.path.exists(db_path):
        print(f"错误: 数据库文件 '{db_path}' 未找到。请确保它与脚本在同一目录下。")
        return []

    records = []
    try:
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()
        
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

def process_and_clip_records(records, target_date):
    """
    处理原始记录，进行时间解析和窗口裁剪。
    """
    processed_data = []
    base_date_obj = datetime.strptime(target_date, '%Y%m%d')
    window_start = base_date_obj
    window_end = base_date_obj + timedelta(days=1)

    for start_str, end_str, project_path in records:
        try:
            start_time = datetime.strptime(start_str, '%H:%M').time()
            end_time = datetime.strptime(end_str, '%H:%M').time()
            
            start_dt = base_date_obj.combine(base_date_obj.date(), start_time)
            end_dt = base_date_obj.combine(base_date_obj.date(), end_time)

            if end_dt < start_dt:
                end_dt += timedelta(days=1)

            plot_start = max(start_dt, window_start)
            plot_end = min(end_dt, window_end)

            if plot_start < plot_end:
                parent_project = project_path.split('_')[0]
                processed_data.append({
                    'label': parent_project,
                    'full_path': project_path,
                    'start': plot_start,
                    'end': plot_end,
                    'color': get_color_for_project(project_path)
                })
        except ValueError as e:
            print(f"警告: 跳过格式错误的时间记录: {start_str}-{end_str}. 错误: {e}")
            continue
    return processed_data, window_start, window_end

def plot_detailed_timeline(plot_data, target_date, window_start, window_end):
    """
    绘制详细视图时间线，每个活动占一行。
    """
    if not plot_data:
        return

    fig, ax = plt.subplots(figsize=(15, max(6, len(plot_data) * 0.5)))
    
    labels = [f"{item['label']} ({item['start'].strftime('%H:%M')}-{item['end'].strftime('%H:%M')})" for item in plot_data]
    y_pos = range(len(plot_data))

    for i, data in enumerate(plot_data):
        start_num = mdates.date2num(data['start'])
        duration_num = mdates.date2num(data['end']) - start_num
        ax.broken_barh([(start_num, duration_num)], (i - 0.4, 0.8), facecolors=data['color'])

    ax.set_yticks(y_pos)
    ax.set_yticklabels(labels)
    ax.invert_yaxis()

    ax.xaxis.set_major_formatter(mdates.DateFormatter('%H:%M'))
    ax.xaxis.set_major_locator(mdates.HourLocator(interval=2))
    ax.xaxis.set_minor_locator(mdates.HourLocator(interval=1))
    ax.set_xlim(window_start, window_end)
    
    ax.set_title(f'{target_date[:4]}年{target_date[4:6]}月{target_date[6:]}日 时间线 [详细视图]', fontsize=16)
    ax.set_xlabel('时间', fontsize=12)
    ax.set_ylabel('活动', fontsize=12)

    ax.grid(True, which='major', axis='x', linestyle='--', linewidth=0.5)
    fig.autofmt_xdate()
    plt.tight_layout()

    output_filename = f'timeline_detailed_{target_date}.png'
    plt.savefig(output_filename, dpi=150)
    print(f"详细视图已成功保存为: {output_filename}")
    plt.close(fig) # 关闭图表以释放内存

def plot_grouped_timeline(plot_data, target_date, window_start, window_end):
    """
    绘制分组视图时间线，同类活动在同一行，并输出统计。
    在每个时长超过30分钟的独立活动片段上标注时长和占比。
    """
    if not plot_data:
        return
        
    grouped_data = defaultdict(list)
    project_durations = defaultdict(timedelta)

    for item in plot_data:
        parent_project = item['label']
        grouped_data[parent_project].append(item)
        project_durations[parent_project] += (item['end'] - item['start'])

    # 首先计算总时长，用于后续百分比计算
    total_duration = sum(project_durations.values(), timedelta())
    if total_duration.total_seconds() == 0:
        print("警告: 总活动时长为零，无法继续绘图。")
        return

    fig, ax = plt.subplots(figsize=(15, max(6, len(grouped_data) * 0.8)))

    y_labels = sorted(grouped_data.keys())
    y_positions = {label: i for i, label in enumerate(y_labels)}

    for label, items in grouped_data.items():
        y_pos = y_positions[label]
        color = get_color_for_project(label)
        segments = [(mdates.date2num(item['start']), mdates.date2num(item['end']) - mdates.date2num(item['start'])) for item in items]
        ax.broken_barh(segments, (y_pos - 0.4, 0.8), facecolors=color)

        # --- 修改后的代码：为每个符合条件的活动片段添加文本标签 ---
        for item in items:
            segment_duration = item['end'] - item['start']
            # 条件：只为持续时间大于30分钟的独立片段添加标签
            if segment_duration > timedelta(minutes=30):
                # 1. 计算百分比
                percentage = (segment_duration.total_seconds() / total_duration.total_seconds()) * 100
                
                # 2. 格式化时长文本 (例如 "1h 30m")
                hours, remainder = divmod(segment_duration.total_seconds(), 3600)
                minutes, _ = divmod(remainder, 60)
                parts = []
                if hours > 0:
                    parts.append(f"{int(hours)}h")
                if minutes > 0:
                    parts.append(f"{int(minutes)}m")
                duration_str = " ".join(parts)
                
                # 3. 组合最终文本 (带换行)
                label_text = f"{duration_str}\n({percentage:.1f}%)"

                # 4. 确定文本位置 (在当前片段的中心)
                text_x_pos_dt = item['start'] + segment_duration / 2
                text_x_pos_num = mdates.date2num(text_x_pos_dt)

                # 5. 在图上添加文本 (白色带黑边，确保在所有颜色上都清晰可见)
                text_effect = [path_effects.withStroke(linewidth=2.5, foreground='black')]
                ax.text(text_x_pos_num, y_pos, label_text, 
                        ha='center', va='center', 
                        fontsize=8, color='white', weight='bold',
                        path_effects=text_effect,
                        linespacing=0.9)

    ax.set_yticks(list(y_positions.values()))
    ax.set_yticklabels(list(y_positions.keys()))
    ax.invert_yaxis()

    ax.xaxis.set_major_formatter(mdates.DateFormatter('%H:%M'))
    ax.xaxis.set_major_locator(mdates.HourLocator(interval=2))
    ax.xaxis.set_minor_locator(mdates.HourLocator(interval=1))
    ax.set_xlim(window_start, window_end)
    
    ax.set_title(f'{target_date[:4]}年{target_date[4:6]}月{target_date[6:]}日 时间线 [分组视图]', fontsize=16)
    ax.set_xlabel('时间', fontsize=12)
    ax.set_ylabel('活动类别', fontsize=12)

    ax.grid(True, which='major', axis='x', linestyle='--', linewidth=0.5)
    fig.autofmt_xdate()
    plt.tight_layout()

    output_filename = f'timeline_grouped_{target_date}.png'
    plt.savefig(output_filename, dpi=150)
    print(f"分组视图已成功保存为: {output_filename}")
    plt.close(fig)

    # --- 更新控制台输出，加入百分比 ---
    print("\n--- 当日活动时长统计 ---")
    sorted_durations = sorted(project_durations.items(), key=lambda item: item[1], reverse=True)
    
    if total_duration.total_seconds() > 0:
        print(f"{'项目':<16}| {'时长':<18} | {'占比'}")
        print("-" * 45)
        for project, duration in sorted_durations:
            hours, remainder = divmod(duration.total_seconds(), 3600)
            minutes, _ = divmod(remainder, 60)
            percentage = (duration.total_seconds() / total_duration.total_seconds()) * 100
            duration_text = f"{int(hours):02d} 小时 {int(minutes):02d} 分钟"
            print(f"{project:<15} | {duration_text:<18} | ({percentage:.1f}%)")
        
        total_hours, total_remainder = divmod(total_duration.total_seconds(), 3600)
        total_minutes, _ = divmod(total_remainder, 60)
        total_duration_text = f"{int(total_hours):02d} 小时 {int(total_minutes):02d} 分钟"
        print("-" * 45)
        print(f"{'总计':<15} | {total_duration_text:<18} | (100.0%)")
    else:
        print("无有效时长的活动。")


def main():
    """
    主执行函数
    """
    configure_matplotlib_for_chinese()

    if len(sys.argv) != 2:
        print("用法: python timeline_generator.py <日期YYYYMMDD>")
        print("例如: python timeline_generator.py 20250401")
        sys.exit(1)

    db_path = 'time_data.db'
    target_date = sys.argv[1]

    if len(target_date) != 8 or not target_date.isdigit():
        print("错误: 日期格式必须为 projetypiMMDD (例如: 20250401)")
        sys.exit(1)
    
    records = fetch_daily_records(db_path, target_date)
    if records:
        plot_data, window_start, window_end = process_and_clip_records(records, target_date)
        if plot_data:
            plot_detailed_timeline(plot_data, target_date, window_start, window_end)
            plot_grouped_timeline(plot_data, target_date, window_start, window_end)
        else:
            print("没有在00:00-24:00时间范围内的有效活动可供绘图。")

if __name__ == "__main__":
    main()
