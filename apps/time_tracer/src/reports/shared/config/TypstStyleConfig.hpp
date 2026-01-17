// reports/shared/config/TypstStyleConfig.hpp
#ifndef REPORTS_SHARED_CONFIG_TYPST_STYLE_CONFIG_HPP_
#define REPORTS_SHARED_CONFIG_TYPST_STYLE_CONFIG_HPP_

#include <string>
#include <toml++/toml.h> // [修改]
#include "reports/shared/api/shared_api.hpp"

DISABLE_C4251_WARNING

class REPORTS_SHARED_API TypstStyleConfig {
public:
    // [修改] 接收 toml::table
    explicit TypstStyleConfig(const toml::table& config);

    const std::string& get_base_font() const;
    const std::string& get_title_font() const;
    const std::string& get_category_title_font() const;
    int get_base_font_size() const;
    int get_report_title_font_size() const;
    int get_category_title_font_size() const;
    double get_line_spacing_em() const;

private:
    std::string base_font_;
    std::string title_font_;
    std::string category_title_font_;
    int base_font_size_;
    int report_title_font_size_;
    int category_title_font_size_;
    double line_spacing_em_;
};

ENABLE_C4251_WARNING

#endif // REPORTS_SHARED_CONFIG_TYPST_STYLE_CONFIG_HPP_