// infrastructure/reports/shared/config/tex_style_config.cpp
#include "infrastructure/reports/shared/config/tex_style_config.hpp"

TexStyleConfig::TexStyleConfig(const toml::table& config) {
  constexpr int kDefaultBaseFontSize = 10;
  constexpr int kDefaultReportTitleFontSize = 14;
  constexpr int kDefaultCategoryTitleFontSize = 12;
  constexpr double kDefaultMarginIn = 1.0;

  // 移除 value_or 后面的 <std::string>，让编译器自动推导类型
  // 这样就可以传递 main_font_ (左值) 作为默认值了
  main_font_ = config["main_font"].value_or("");
  cjk_main_font_ = config["cjk_main_font"].value_or(main_font_);

  base_font_size_ = config["base_font_size"].value_or(kDefaultBaseFontSize);
  report_title_font_size_ =
      config["report_title_font_size"].value_or(kDefaultReportTitleFontSize);
  category_title_font_size_ = config["category_title_font_size"].value_or(
      kDefaultCategoryTitleFontSize);
  margin_in_ = config["margin_in"].value_or(kDefaultMarginIn);
  list_top_sep_pt_ = config["list_top_sep_pt"].value_or(0.0);
  list_item_sep_ex_ = config["list_item_sep_ex"].value_or(0.0);
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
