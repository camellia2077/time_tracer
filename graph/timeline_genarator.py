import sys
import os
import sqlite3
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import numpy as np

def format_duration(seconds: float) -> str:
    """将秒格式化为易于阅读的字符串（如 H:M 或 M:S）。"""
    if seconds is None or seconds < 0:
        return "0s"
    seconds = int(seconds)
    hours = seconds // 3600
    minutes = (seconds % 3600) // 60
    secs = seconds % 60
    if hours > 0:
        return f"{hours}h {minutes}m"
    if minutes > 0:
        return f"{minutes}m {secs}s"
    return f"{secs}s"

def sort_tree(node: dict):
    """递归地对树中每个节点的子节点按时长降序排序。"""
    if 'children' in node and node['children']:
        node['children'].sort(key=lambda x: x['duration'], reverse=True)
        for child in node['children']:
            sort_tree(child)

def get_max_depth(node: dict) -> int:
    """递归计算树的最大深度。"""
    if not node.get('children'):
        return 1
    return 1 + max(get_max_depth(child) for child in node['children'])

def draw_icicle(ax, node: dict, x: float, y: float, width: float, height: float, base_cmap, level: int):
    """
    递归绘制冰柱图的函数。
    它会绘制给定节点的所有子节点，并使用继承的色系。
    """
    children = node.get('children', [])
    if not children:
        return

    total_child_duration = sum(c['duration'] for c in children)
    if total_child_duration == 0:
        return

    # 创建一个归一化函数，将时长映射到颜色范围 [0.3, 0.9]
    # 以避免颜色过浅（看不清文字）或过深（无法区分）
    child_durations = [c['duration'] for c in children]
    min_dur, max_dur = min(child_durations), max(child_durations)
    
    def normalize_duration(duration):
        if max_dur == min_dur:
            return 0.7  # 如果所有子项时长相同，使用中等色调
        # 线性映射到 [0.3, 0.9]
        return 0.3 + 0.6 * ((duration - min_dur) / (max_dur - min_dur))

    current_y = y

    for child in children:
        child_height = (child['duration'] / total_child_duration) * height
        
        # 根据归一化后的时长从基础色系中获取颜色
        color_value = normalize_duration(child['duration'])
        color = base_cmap(color_value)

        # 创建并添加矩形
        rect = patches.Rectangle(
            (x, current_y), width, child_height,
            linewidth=1.5, edgecolor='white', facecolor=color
        )
        ax.add_patch(rect)

        # 在矩形中心添加标签
        if child_height > 0.01 * ax.get_ylim()[1]:
            label = f"{child['name']}\n{format_duration(child['duration'])}"
            text_color = 'white' if sum(color[:3]) * 255 < 382 else 'black'
            ax.text(x + width / 2, current_y + child_height / 2, label,
                    ha='center', va='center', fontsize=8, color=text_color, wrap=True)

        # 递归为孙子节点绘图，传递相同的色系
        draw_icicle(ax, child, x + width, current_y, width, child_height, base_cmap, level + 1)
        current_y += child_height

def main():
    """主函数，用于解析参数、获取数据、构建树和绘图。"""
    db_file = 'time_data.db'
    if not os.path.exists(db_file):
        print(f"错误: 数据库文件 '{db_file}' 在当前目录中不存在。")
        return

    if len(sys.argv) != 2:
        print("用法: python plot_timeline.py <YYYY|YYYYMM|YYYYMMDD>")
        return

    date_arg = sys.argv[1]
    if len(date_arg) == 4 and date_arg.isdigit():
        date_pattern, title_period = f"{date_arg}%", f"{date_arg}年"
    elif len(date_arg) == 6 and date_arg.isdigit():
        date_pattern, title_period = f"{date_arg}%", f"{date_arg[:4]}年{date_arg[4:]}月"
    elif len(date_arg) == 8 and date_arg.isdigit():
        date_pattern, title_period = date_arg, f"{date_arg[:4]}年{date_arg[4:6]}月{date_arg[6:]}日"
    else:
        print("错误: 日期参数格式不正确。请使用 YYYY, YYYYMM, 或 YYYYMMDD 格式。")
        return

    try:
        conn = sqlite3.connect(db_file)
        df_records = pd.read_sql_query("SELECT project_path, duration FROM time_records WHERE date LIKE ?", conn, params=(date_pattern,))
        if df_records.empty:
            print(f"在指定的时间段 '{title_period}' 内没有找到任何时间记录。")
            return
        df_hierarchy = pd.read_sql_query("SELECT child, parent FROM parent_child", conn)
    except sqlite3.Error as e:
        print(f"数据库错误: {e}")
        return
    finally:
        if 'conn' in locals() and conn:
            conn.close()

    # --- 数据处理 ---
    parent_map = pd.Series(df_hierarchy.parent.values, index=df_hierarchy.child).to_dict()
    durations = {}
    for _, row in df_records.iterrows():
        path, duration = row['project_path'], row['duration']
        current_path = path
        while current_path:
            durations[current_path] = durations.get(current_path, 0) + duration
            current_path = parent_map.get(current_path)
    
    if not durations:
        print(f"在 '{title_period}' 内的记录没有有效的项目路径或时长。")
        return

    nodes = {path: {'name': path.split('_')[-1], 'path': path, 'duration': d, 'children': []} for path, d in durations.items()}
    
    # 建立初步的父子关系
    initial_root_nodes = []
    for path, node in nodes.items():
        parent_path = parent_map.get(path)
        if parent_path and parent_path in nodes:
            nodes[parent_path]['children'].append(node)
        else:
            initial_root_nodes.append(node)

    # 合并大小写根节点 (例如 'study' 合并到 'STUDY')
    merged_roots = {}
    for node in initial_root_nodes:
        key = node['path'].upper()
        if key in merged_roots:
            merged_roots[key]['duration'] += node['duration']
            merged_roots[key]['children'].extend(node['children'])
        else:
            new_node = node.copy()
            new_node['name'] = new_node['path'].upper().split('_')[-1]
            merged_roots[key] = new_node
    
    root_nodes = list(merged_roots.values())
    
    # 按时长对顶级父项排序
    root_nodes.sort(key=lambda x: x['duration'], reverse=True)

    total_duration = df_records['duration'].sum()
    root = {'name': 'Total', 'path': 'Total', 'duration': total_duration, 'children': root_nodes}

    # 对所有层级的子项进行排序
    sort_tree(root)

    # --- 绘图 ---
    fig, ax = plt.subplots(figsize=(18, 10))
    plt.rcParams['font.sans-serif'] = ['SimHei', 'Heiti TC', 'sans-serif']
    plt.rcParams['axes.unicode_minus'] = False

    # 绘制“总时间”柱
    ax.add_patch(patches.Rectangle((0, 0), 1, total_duration, linewidth=2, edgecolor='black', facecolor='#E0E0E0'))
    ax.text(0.5, total_duration / 2, f"总时间\n{format_duration(total_duration)}", ha='center', va='center', fontsize=12, color='black', weight='bold')

    # 为每个顶级父项分配不同的色系并绘制
    cmap_list = [plt.get_cmap(name) for name in ['Blues', 'Greens', 'Oranges', 'Reds', 'Purples', 'YlOrBr', 'pink', 'coolwarm']]
    top_level_parents = root['children']
    current_y = 0

    if root['duration'] > 0:
        for i, parent_node in enumerate(top_level_parents):
            branch_cmap = cmap_list[i % len(cmap_list)]
            parent_height = (parent_node['duration'] / root['duration']) * total_duration
            
            # 绘制第二列的父项矩形
            rect_color = branch_cmap(0.7)
            rect = patches.Rectangle((1, current_y), 1, parent_height, linewidth=1.5, edgecolor='white', facecolor=rect_color)
            ax.add_patch(rect)
            
            label_text = f"{parent_node['name']}\n{format_duration(parent_node['duration'])}"
            text_color = 'white' if sum(rect_color[:3]) * 255 < 382 else 'black'
            ax.text(1.5, current_y + parent_height / 2, label_text, ha='center', va='center', fontsize=9, color=text_color, wrap=True)

            # 从第三列开始，递归绘制其子项
            if parent_node['children']:
                draw_icicle(ax, parent_node, 2, current_y, 1, parent_height, branch_cmap, level=2)
            current_y += parent_height

    # 设置图表样式
    ax.set_title(f'{title_period} 时间耗时冰柱图', fontsize=18, pad=20)
    ax.set_yticks([])
    ax.set_xticks([])
    for spine in ax.spines.values():
        spine.set_visible(False)
    
    max_depth = get_max_depth(root) if root['children'] else 1
    ax.set_xlim(-0.5, max_depth + 1.5)
    ax.set_ylim(0, total_duration * 1.05)
    
    plt.tight_layout()
    plt.show()

if __name__ == '__main__':
    main()
