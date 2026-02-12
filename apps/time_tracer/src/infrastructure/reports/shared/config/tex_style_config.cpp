// infrastructure/reports/shared/config/tex_style_config.cpp
#include "infrastructure/reports/shared/config/tex_style_config.hpp"

#include "infrastructure/reports/shared/interfaces/formatter_c_string_view_utils.hpp"

TexStyleConfig::TexStyleConfig(const TtTexStyleConfigV1& config) {
  main_font_ = formatter_c_string_view_utils::ToString(config.mainFont,
                                                       "style.mainFont");
  cjk_main_font_ = formatter_c_string_view_utils::ToString(config.cjkMainFont,
                                                           "style.cjkMainFont");
  base_font_size_ = config.baseFontSize;
  report_title_font_size_ = config.reportTitleFontSize;
  category_title_font_size_ = config.categoryTitleFontSize;
  margin_in_ = config.marginIn;
  list_top_sep_pt_ = config.listTopSepPt;
  list_item_sep_ex_ = config.listItemSepEx;
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
