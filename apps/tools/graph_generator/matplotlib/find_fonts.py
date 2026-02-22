from matplotlib import font_manager

def find_system_fonts():
    """
    查找并打印 Matplotlib 在当前系统上能找到的所有字体。
    """
    print("--- Matplotlib 可用字体列表 ---")

    # 获取所有找到的字体文件路径
    font_paths = font_manager.findSystemFonts(fontpaths=None, fontext='ttf')
    font_paths.extend(font_manager.findSystemFonts(fontpaths=None, fontext='otf'))

    if not font_paths:
        print("错误：在系统中没有找到字体文件。")
        return

    # 使用集合来存储唯一的字体名称，以避免重复打印
    unique_font_names = set()

    for font_path in font_paths:
        try:
            # 从字体文件路径创建字体属性对象
            font_prop = font_manager.FontProperties(fname=font_path)
            # 获取字体的正式名称
            font_name = font_prop.get_name()
            unique_font_names.add(font_name)
        except Exception:
            # 某些字体文件可能已损坏，忽略它们
            continue

    # 按字母顺序排序并打印
    sorted_font_names = sorted(list(unique_font_names))

    for name in sorted_font_names:
        print(name)

    print(f"\n--- 总共找到 {len(sorted_font_names)} 个独立字体 ---")

if __name__ == '__main__':
    find_system_fonts()