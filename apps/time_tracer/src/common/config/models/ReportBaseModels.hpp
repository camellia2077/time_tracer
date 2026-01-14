// common/config/models/ReportBaseModels.hpp
#ifndef REPORT_BASE_MODELS_HPP
#define REPORT_BASE_MODELS_HPP

#include <string>
#include <vector>

// 统计项定义 (支持递归子项)
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

    int base_font_size = 10;
    int report_title_font_size = 14;
    int category_title_font_size = 12;
};

// 布局配置
struct LayoutConfig {
    // LaTeX
    double margin_in = 1.0;
    double list_top_sep_pt = 0.0;
    double list_item_sep_ex = 0.0;

    // Typst
    double line_spacing_em = 0.65;
    double margin_top_cm = 2.5;
    double margin_bottom_cm = 2.5;
    double margin_left_cm = 2.0;
    double margin_right_cm = 2.0;
};

#endif // REPORT_BASE_MODELS_HPP