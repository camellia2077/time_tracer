// reports/shared/config/typst_style_config.cpp
#include "typst_style_config.hpp"

TypstStyleConfig::TypstStyleConfig(const toml::table& tbl) {
  // 移除 value_or 后的 <std::string>
  // toml++ 会根据传入参数自动推导返回类型，并支持左值引用
  base_font_ = tbl["base_font"].value_or("");
  title_font_ = tbl["title_font"].value_or(base_font_);
  category_title_font_ = tbl["category_title_font"].value_or(base_font_);

  base_font_size_ = tbl["base_font_size"].value_or(10);
  report_title_font_size_ = tbl["report_title_font_size"].value_or(14);
  category_title_font_size_ = tbl["category_title_font_size"].value_or(12);
  line_spacing_em_ = tbl["line_spacing_em"].value_or(0.65);
}

auto TypstStyleConfig::get_base_font() const -> const std::string& {
  return base_font_;
}
auto TypstStyleConfig::get_title_font() const -> const std::string& {
  return title_font_;
}
auto TypstStyleConfig::get_category_title_font() const -> const std::string& {
  return category_title_font_;
}
auto TypstStyleConfig::get_base_font_size() const -> int {
  return base_font_size_;
}
auto TypstStyleConfig::get_report_title_font_size() const -> int {
  return report_title_font_size_;
}
auto TypstStyleConfig::get_category_title_font_size() const -> int {
  return category_title_font_size_;
}
auto TypstStyleConfig::get_line_spacing_em() const -> double {
  return line_spacing_em_;
}