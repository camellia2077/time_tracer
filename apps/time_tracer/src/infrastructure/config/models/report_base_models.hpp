// infrastructure/config/models/models/report_base_models.hpp
#ifndef COMMON_CONFIG_MODELS_REPORT_BASE_MODELS_H_
#define COMMON_CONFIG_MODELS_REPORT_BASE_MODELS_H_

#include <string>
#include <vector>

// 统计项定义
struct ReportStatisticsItem {
  std::string label;
  bool show = true;
  std::string db_column;
  std::vector<ReportStatisticsItem> sub_items;
};

// 字体配置
struct FontConfig {
  std::string base_font;
  std::string title_font;
  std::string category_title_font;
  std::string main_font;      // LaTeX specific
  std::string cjk_main_font;  // LaTeX specific

  static constexpr int kDefaultBaseFontSize = 10;
  static constexpr int kDefaultTitleFontSize = 14;
  static constexpr int kDefaultCategoryFontSize = 12;

  int base_font_size = kDefaultBaseFontSize;
  int report_title_font_size = kDefaultTitleFontSize;
  int category_title_font_size = kDefaultCategoryFontSize;
};

// 布局配置
struct LayoutConfig {
  // LaTeX
  double margin_in = 1.0;
  double list_top_sep_pt = 0.0;
  double list_item_sep_ex = 0.0;

  static constexpr double kDefaultLineSpacing = 0.65;
  static constexpr double kVerticalMarginCm = 2.5;
  static constexpr double kHorizontalMarginCm = 2.0;

  // Typst
  double line_spacing_em = kDefaultLineSpacing;
  double margin_top_cm = kVerticalMarginCm;
  double margin_bottom_cm = kVerticalMarginCm;
  double margin_left_cm = kHorizontalMarginCm;
  double margin_right_cm = kHorizontalMarginCm;
};

#endif  // COMMON_CONFIG_MODELS_REPORT_BASE_MODELS_H_