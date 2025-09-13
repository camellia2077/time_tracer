import matplotlib
import os

try:
    # 获取 matplotlib 缓存目录的路径
    cache_dir = matplotlib.get_cachedir()
    print(f"Matplotlib 缓存目录是: {cache_dir}")

    # 查找并删除字体缓存文件 (通常是 fontlist-vXXX.json)
    font_cache_file = os.path.join(cache_dir, 'fontlist-v330.json') # 版本号可能不同

    # 更稳妥的方式是遍历目录查找
    for filename in os.listdir(cache_dir):
        if filename.startswith('fontlist') and filename.endswith('.json'):
            font_cache_path = os.path.join(cache_dir, filename)
            print(f"正在删除旧的字体缓存文件: {font_cache_path}")
            os.remove(font_cache_path)
            print("✅ 缓存已成功删除。")
            print("下次运行任何 matplotlib 脚本时，缓存将自动重建。")
            break
    else:
        print("没有找到字体缓存文件 (可能已经被清理)。")

except Exception as e:
    print(f"发生错误: {e}")
    print("请尝试手动删除 matplotlib 缓存目录下的 'fontlist-vXXX.json' 文件。")