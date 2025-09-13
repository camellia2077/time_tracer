# timeline_generator.py (Final Version with Generic Font)

import sqlite3
import tomllib
import os
from datetime import datetime

import matplotlib.pyplot as plt
import matplotlib.dates as mdates

def load_config(config_path="config.toml"):
    """从 TOML 文件加载配置。"""
    try:
        with open(config_path, "rb") as f:
            return tomllib.load(f)
    except FileNotFoundError:
        print(f"错误: 配置文件 '{config_path}' 未找到。")
        exit(1)
    except tomllib.TOMLDecodeError as e:
        print(f"错误: 解析配置文件 '{config_path}' 失败: {e}")
        exit(1)

def get_activity_color(project_path, color_map, default_color):
    """根据 project_path 的顶级父级获取颜色。"""
    top_parent = project_path.split('_')[0]
    return color_map.get(top_parent, default_color)

def create_timeline_chart(config):
    """连接数据库，查询数据，并根据配置生成时间线图表。"""
    db_path = config["paths"]["database"]
    query_date = config["settings"]["target_date"]
    output_dir = config["paths"]["output_directory"]
    colors = config["colors"]
    default_color = colors.get("default", "#808080")
    
    # ✨ 核心修改：不再从 config 读取字体，直接设置通用字体
    # Matplotlib 会自动选择一个可用的 sans-serif 字体
    plt.rcParams['font.family'] = 'sans-serif'
    # ✨ 新增：指定一个通用的黑体来显示中文，以防乱码
    plt.rcParams['font.sans-serif'] = ['SimHei'] 
    plt.rcParams['axes.unicode_minus'] = False

    conn = None
    try:
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()

        sql_query = """
        SELECT start_timestamp, end_timestamp, project_path 
        FROM time_records 
        WHERE date = ? 
        ORDER BY logical_id ASC;
        """
        cursor.execute(sql_query, (query_date,))
        records = cursor.fetchall()

        if not records:
            print(f"在日期 {query_date} 没有找到任何活动记录。")
            return

        fig, ax = plt.subplots(figsize=(18, 10))
        y_labels = []
        y_positions = []

        for i, record in enumerate(records):
            start_timestamp, end_timestamp, project_path = record
            
            start_dt = datetime.fromtimestamp(start_timestamp)
            end_dt = datetime.fromtimestamp(end_timestamp)

            start_num = mdates.date2num(start_dt)
            end_num = mdates.date2num(end_dt)
            duration_num = end_num - start_num

            bar_color = get_activity_color(project_path, colors, default_color)
            ax.broken_barh([(start_num, duration_num)], (i - 0.4, 0.8), facecolors=bar_color, edgecolors='grey')
            
            text_x = start_num + duration_num / 2
            text_y = i
            duration_total_seconds = end_timestamp - start_timestamp
            duration_hours = int(duration_total_seconds / 3600)
            duration_minutes = int((duration_total_seconds % 3600) / 60)
            duration_text = f"{duration_hours}h {duration_minutes}m"
            ax.text(text_x, text_y, duration_text, ha='center', va='center', color='black', fontsize=9, weight='bold')

            y_labels.append(project_path)
            y_positions.append(i)

        # --- 格式化图表 ---
        ax.set_yticks(y_positions)
        ax.set_yticklabels(y_labels, fontsize=12)
        ax.set_ylim(-0.5, len(records) - 0.5)
        ax.set_ylabel('活动名称 (Project Path)', fontsize=14, weight='bold')

        ax.xaxis_date()
        date_formatter = mdates.DateFormatter('%H:%M')
        ax.xaxis.set_major_formatter(date_formatter)
        ax.xaxis.set_major_locator(plt.MaxNLocator(24))
        plt.setp(ax.get_xticklabels(), rotation=45, ha="right", fontsize=10)

        ax.set_xlabel('一天内的时间 (开始 - 结束)', fontsize=14, weight='bold')
        plt.title(f'{query_date} 活动时间线', fontsize=18, weight='bold')
        plt.grid(True, which='major', axis='x', linestyle='--', linewidth=0.7)
        plt.tight_layout()

        if not os.path.exists(output_dir):
            os.makedirs(output_dir)
            print(f"已创建输出目录: {output_dir}")

        output_filename = f"{query_date}_timeline.png"
        output_path = os.path.join(output_dir, output_filename)
        
        plt.savefig(output_path, dpi=300)
        print(f"图表已成功保存到: {output_path}")

    except sqlite3.Error as e:
        print(f"数据库查询出错: {e}")
    finally:
        if conn:
            conn.close()

if __name__ == '__main__':
    config_data = load_config()
    create_timeline_chart(config_data)