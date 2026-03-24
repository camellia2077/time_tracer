// infra/reporting/shared/config/typst_style_config.cpp
#include "infra/reporting/shared/config/typst_style_config.hpp"

TypstStyleConfig::TypstStyleConfig(const FontConfig& fonts,
                                   const LayoutConfig& layout) {
  base_font_ = fonts.base_font;
  title_font_ = fonts.title_font;
  category_title_font_ = fonts.category_title_font;
  base_font_size_ = fonts.base_font_size;
  report_title_font_size_ = fonts.report_title_font_size;
  category_title_font_size_ = fonts.category_title_font_size;
  line_spacing_em_ = layout.line_spacing_em;
}

auto TypstStyleConfig::GetBaseFont() const -> const std::string& {
  return base_font_;
}
auto TypstStyleConfig::GetTitleFont() const -> const std::string& {
  return title_font_;
}
auto TypstStyleConfig::GetCategoryTitleFont() const -> const std::string& {
  return category_title_font_;
}
auto TypstStyleConfig::GetBaseFontSize() const -> int {
  return base_font_size_;
}
auto TypstStyleConfig::GetReportTitleFontSize() const -> int {
  return report_title_font_size_;
}
auto TypstStyleConfig::GetCategoryTitleFontSize() const -> int {
  return category_title_font_size_;
}
auto TypstStyleConfig::GetLineSpacingEm() const -> double {
  return line_spacing_em_;
}
