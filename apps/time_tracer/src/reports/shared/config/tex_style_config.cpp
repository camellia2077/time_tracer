// reports/shared/config/tex_style_config.cpp
#include "tex_style_config.hpp"

TexStyleConfig::TexStyleConfig(const toml::table& tbl) {
  // 移除 value_or 后面的 <std::string>，让编译器自动推导类型
  // 这样就可以传递 main_font_ (左值) 作为默认值了
  main_font_ = tbl["main_font"].value_or("");
  cjk_main_font_ = tbl["cjk_main_font"].value_or(main_font_);

  base_font_size_ = tbl["base_font_size"].value_or(10);
  report_title_font_size_ = tbl["report_title_font_size"].value_or(14);
  category_title_font_size_ = tbl["category_title_font_size"].value_or(12);
  margin_in_ = tbl["margin_in"].value_or(1.0);
  list_top_sep_pt_ = tbl["list_top_sep_pt"].value_or(0.0);
  list_item_sep_ex_ = tbl["list_item_sep_ex"].value_or(0.0);
}

auto TexStyleConfig::get_main_font() const -> const std::string& {
  return main_font_;
}
auto TexStyleConfig::get_cjk_main_font() const -> const std::string& {
  return cjk_main_font_;
}
auto TexStyleConfig::get_base_font_size() const -> int {
  return base_font_size_;
}
auto TexStyleConfig::get_report_title_font_size() const -> int {
  return report_title_font_size_;
}
auto TexStyleConfig::get_category_title_font_size() const -> int {
  return category_title_font_size_;
}
auto TexStyleConfig::get_margin_in() const -> double {
  return margin_in_;
}
auto TexStyleConfig::get_list_top_sep_pt() const -> double {
  return list_top_sep_pt_;
}
auto TexStyleConfig::get_list_item_sep_ex() const -> double {
  return list_item_sep_ex_;
}