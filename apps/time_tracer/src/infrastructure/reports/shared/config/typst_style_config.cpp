// infrastructure/reports/shared/config/typst_style_config.cpp
#include "infrastructure/reports/shared/config/typst_style_config.hpp"

#include "infrastructure/reports/shared/interfaces/formatter_c_string_view_utils.hpp"

TypstStyleConfig::TypstStyleConfig(const TtTypstStyleConfigV1& config) {
  base_font_ = formatter_c_string_view_utils::ToString(config.baseFont,
                                                       "style.baseFont");
  title_font_ = formatter_c_string_view_utils::ToString(config.titleFont,
                                                        "style.titleFont");
  category_title_font_ = formatter_c_string_view_utils::ToString(
      config.categoryTitleFont, "style.categoryTitleFont");
  base_font_size_ = config.baseFontSize;
  report_title_font_size_ = config.reportTitleFontSize;
  category_title_font_size_ = config.categoryTitleFontSize;
  line_spacing_em_ = config.lineSpacingEm;
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
