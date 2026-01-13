// reports/shared/formatters/templates/BaseTypFormatter.hpp
#ifndef BASE_TYP_FORMATTER_HPP
#define BASE_TYP_FORMATTER_HPP

#include <string>
#include <sstream>
#include <memory>
#include <format>
#include "reports/shared/interfaces/IReportFormatter.hpp"
#include "reports/shared/formatters/typst/TypUtils.hpp"

/**
 * @class BaseTypFormatter
 * @brief Typst 报告格式化器的通用模板基类。
 */
template <typename ReportDataT, typename ConfigT>
class BaseTypFormatter : public IReportFormatter<ReportDataT> {
public:
    explicit BaseTypFormatter(std::shared_ptr<ConfigT> config) : config_(config) {}

    std::string format_report(const ReportDataT& data) const override {
        std::stringstream ss;

        // 1. 页面和基础文本设置
        format_page_setup(ss);
        format_text_setup(ss);

        // 2. 数据有效性检查
        if (std::string err = validate_data(data); !err.empty()) {
            ss << err << "\n";
            return ss.str();
        }

        // 3. 头部 / 摘要
        format_header_content(ss, data);

        // 4. 主体内容
        if (is_empty_data(data)) {
            // [修改] 调用纯虚函数
            ss << get_no_records_msg() << "\n";
        } else {
            format_extra_content(ss, data);
            format_project_tree_section(ss, data);
        }

        return ss.str();
    }

protected:
    std::shared_ptr<ConfigT> config_;

    // [修改] 注释掉未使用参数
    virtual std::string validate_data(const ReportDataT& /*data*/) const { return ""; }
    
    virtual bool is_empty_data(const ReportDataT& data) const = 0;
    virtual int get_avg_days(const ReportDataT& data) const = 0;
    
    // [新增] 纯虚函数
    virtual std::string get_no_records_msg() const = 0;

    virtual void format_header_content(std::stringstream& ss, const ReportDataT& data) const = 0;
    
    // [修改] 注释掉未使用参数
    virtual void format_extra_content(std::stringstream& /*ss*/, const ReportDataT& /*data*/) const {}

    // [修改] 注释掉未使用参数
    virtual void format_page_setup(std::stringstream& /*ss*/) const {}

    virtual void format_text_setup(std::stringstream& ss) const {
        std::string spacing_str = std::to_string(config_->get_line_spacing_em()) + "em";
        ss << std::format(R"(#set text(font: "{}", size: {}pt, spacing: {}))",
            config_->get_base_font(),
            config_->get_base_font_size(),
            spacing_str
        ) << "\n\n";
    }

    virtual void format_project_tree_section(std::stringstream& ss, const ReportDataT& data) const {
        // 统领性标题
        ss << std::format(R"(#text(font: "{}", size: {}pt)[= {}])",
            config_->get_category_title_font(),
            config_->get_category_title_font_size(),
            config_->get_project_breakdown_label()
        ) << "\n\n";

        ss << TypUtils::format_project_tree(
            data.project_tree,
            data.total_duration,
            get_avg_days(data),
            config_->get_category_title_font(),
            config_->get_category_title_font_size()
        );
    }
};

#endif // BASE_TYP_FORMATTER_HPP