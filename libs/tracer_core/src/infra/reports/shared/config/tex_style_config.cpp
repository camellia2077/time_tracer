// infra/reports/shared/config/tex_style_config.cpp
#include "infra/reports/shared/config/tex_style_config.hpp"

TexStyleConfig::TexStyleConfig(const FontConfig& fonts,
                               const LayoutConfig& layout) {
  main_font_ = fonts.main_font;
  cjk_main_font_ = fonts.cjk_main_font;
  base_font_size_ = fonts.base_font_size;
  report_title_font_size_ = fonts.report_title_font_size;
  category_title_font_size_ = fonts.category_title_font_size;
  margin_in_ = layout.margin_in;
  list_top_sep_pt_ = layout.list_top_sep_pt;
  list_item_sep_ex_ = layout.list_item_sep_ex;
}

auto TexStyleConfig::GetMainFont() const -> const std::string& {
  return main_font_;
}
auto TexStyleConfig::GetCjkMainFont() const -> const std::string& {
  return cjk_main_font_;
}
auto TexStyleConfig::GetBaseFontSize() const -> int {
  return base_font_size_;
}
auto TexStyleConfig::GetReportTitleFontSize() const -> int {
  return report_title_font_size_;
}
auto TexStyleConfig::GetCategoryTitleFontSize() const -> int {
  return category_title_font_size_;
}
auto TexStyleConfig::GetMarginIn() const -> double {
  return margin_in_;
}
auto TexStyleConfig::GetListTopSepPt() const -> double {
  return list_top_sep_pt_;
}
auto TexStyleConfig::GetListItemSepEx() const -> double {
  return list_item_sep_ex_;
}
