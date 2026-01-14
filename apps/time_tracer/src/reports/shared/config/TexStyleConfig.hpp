// reports/shared/config/TexStyleConfig.hpp
#ifndef TEX_STYLE_CONFIG_HPP
#define TEX_STYLE_CONFIG_HPP

#include <string>
#include <nlohmann/json.hpp>
#include "reports/shared/api/shared_api.hpp"

// 禁用 C4251 警告，因为我们将导出带有 STL 成员的类
DISABLE_C4251_WARNING
// 这个类封装了 LaTeX 报告共有的字体、边距和列表间距设置
class REPORTS_SHARED_API TexStyleConfig {
public:
    explicit TexStyleConfig(const nlohmann::json& json);

    const std::string& get_main_font() const;
    const std::string& get_cjk_main_font() const;
    int get_base_font_size() const;
    int get_report_title_font_size() const;
    int get_category_title_font_size() const;
    double get_margin_in() const;
    double get_list_top_sep_pt() const;
    double get_list_item_sep_ex() const;

private:
    std::string main_font_;
    std::string cjk_main_font_;
    int base_font_size_;
    int report_title_font_size_;
    int category_title_font_size_;
    double margin_in_;
    double list_top_sep_pt_;
    double list_item_sep_ex_;
};

ENABLE_C4251_WARNING

#endif // TEX_STYLE_CONFIG_HPP