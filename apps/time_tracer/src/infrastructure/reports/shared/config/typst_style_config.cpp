// infrastructure/reports/shared/config/typst_style_config.cpp
#include "infrastructure/reports/shared/config/typst_style_config.hpp"

TypstStyleConfig::TypstStyleConfig(const toml::table& config) {
  constexpr int kDefaultBaseFontSize = 10;
  constexpr int kDefaultReportTitleFontSize = 14;
  constexpr int kDefaultCategoryTitleFontSize = 12;
  constexpr double kDefaultLineSpacingEm = 0.65;

  // 移除 value_or 后的 <std::string>
  // toml++ 会根据传入参数自动推导返回类型，并支持左值引用
  base_font_ = config["base_font"].value_or("");
  title_font_ = config["title_font"].value_or(base_font_);
  category_title_font_ = config["category_title_font"].value_or(base_font_);

  base_font_size_ = config["base_font_size"].value_or(kDefaultBaseFontSize);
  report_title_font_size_ =
      config["report_title_font_size"].value_or(kDefaultReportTitleFontSize);
  category_title_font_size_ = config["category_title_font_size"].value_or(
      kDefaultCategoryTitleFontSize);
  line_spacing_em_ = config["line_spacing_em"].value_or(kDefaultLineSpacingEm);
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
