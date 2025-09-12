// 从外部文件导入所有字体列表的定义
#import "font.typ": *

// --- 全局样式控制变量 ---
#let title_size = 16pt      // 字体类型标题的大小
#let name_size = 6pt       // 左列主字体名称的大小
#let sentence_size = 14pt     // 可变字体的字体大小

#let variant_name_size = 6pt // 变体字体名称的大小
#let variant_sentence_size = 14pt // 变体示例文本的大小

// 这个列表用于测试“静态字体”
#let static_variants_to_check = (
  (name: "Regular", weight: "regular", style: "normal"),
  (name: "Italic", weight: "regular", style: "italic"),
  (name: "Bold", weight: "bold", style: "normal"),
  (name: "Bold Italic", weight: "bold", style: "italic"),
)

// 将所有四个方向的边距从 2cm 减小到 1cm 修改margin的值
#set page(width: 24cm, height: auto, margin: 0.5cm)
#set text(size: 11pt)

// 定义要显示的示例文本
#let sentence = "The quick brown fox jumps over the lazy dog."

// --- 【修改后的孙函数】：只负责渲染字体变体的小表格 ---
// 在每个变体后面增加了一行全大写的版本
#let render_variants_table(font_name) = {
  grid(
    columns: (auto, 1fr),
    row-gutter: 8pt,
    column-gutter: 1.5em,
    ..for variant in static_variants_to_check {
      (
        align(right, text(size: variant_name_size, style: "italic")[#variant.name:]),
        text(
          font: font_name,
          size: variant_sentence_size,
          weight: variant.weight,
        style: variant.style,
        )[
          #sentence
          
          #upper(sentence)
        ]
      )
    }
  )
}


// --- 【优化后的辅助函数】：结构更清晰 ---
#let render_static_font_entry(font_name) = {
  grid(
    columns: (auto, 1fr),
    gutter: 1em,
    // 左列：主字体名称
    text(font: "Noto Serif SC", weight: "bold", size: name_size)[#font_name:],
    // 右列：调用孙函数来渲染变体表格
    render_variants_table(font_name)
  )
  v(1.5em)
}

// --- 辅助函数 2 (保持不变) ---
#let render_single_font_entry(font_name) = {
  grid(
    columns: (auto, 1fr),
    gutter: 1em,
  text(font: "Noto Serif SC", weight: "bold", size: name_size)[#font_name:],
    text(font: font_name, size: sentence_size)[#sentence],
  )
  v(0.8em)
}

// --- 主函数 1 (保持不变) ---
#let show_font_list(title, font_list) = {
  v(1.5em)
  text(size: title_size, weight: "bold")[#title]
  v(0.5em)

  for font_name in font_list {
    render_static_font_entry(font_name)
  }
}

// --- 主函数 2 (保持不变) ---
#let show_single_variant(title, font_list) = {
  v(1.5em)
  text(size: title_size, weight: "bold")[#title]
  v(0.5em)

  for font_name in font_list {
    render_single_font_entry(font_name)
  }
}

// --- 主文档内容 (保持不变) ---
#heading(level: 1, "字体分类展示")

#line(length: 100%)
#show_font_list("我最喜欢的字体 (My Favorite)", my_favorite_fonts)


/*
#show_single_variant("可变字体 (Variable Fonts)", variable_fonts)

#show_font_list("展示/装饰体 (Display/Decorative)", display_decorative_fonts)
#show_font_list("等宽字体 (Monospaced)", monospaced_fonts)
#show_font_list("衬线体 (Serif)", serif_fonts)
#show_font_list("无衬线体 (Sans-serif)", sans_serif_fonts)
#show_font_list("粗衬线体 (Slab Serif)", slab_serif_fonts)

#show_font_list("手写体 (Script/Handwriting)", script_handwriting_fonts)

#show_font_list("特殊用途字体 (Special-Purpose)", special_purpose_fonts)
*/
