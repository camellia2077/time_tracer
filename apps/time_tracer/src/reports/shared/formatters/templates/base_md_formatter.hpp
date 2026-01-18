// reports/shared/formatters/templates/base_md_formatter.hpp
#ifndef REPORTS_SHARED_FORMATTERS_TEMPLATES_BASE_MD_FORMATTER_HPP_
#define REPORTS_SHARED_FORMATTERS_TEMPLATES_BASE_MD_FORMATTER_HPP_

#include <string>
#include <sstream>
#include <memory>
#include <format>
#include "reports/shared/interfaces/i_report_formatter.hpp"
#include "reports/shared/formatters/markdown/markdown_formatter.hpp"

/**
 * @class BaseMdFormatter
 * @brief Markdown 报告格式化器的通用模板基类。
 */
template <typename ReportDataT, typename ConfigT>
class BaseMdFormatter : public IReportFormatter<ReportDataT> {
public:
    explicit BaseMdFormatter(std::shared_ptr<ConfigT> config) : config_(config) {}

    std::string format_report(const ReportDataT& data) const override {
        // 1. 数据有效性检查
        if (std::string err = validate_data(data); !err.empty()) {
            return err + "\n"; // Markdown 通常多加个换行比较安全
        }

        std::stringstream ss;

        // 2. 头部 / 摘要
        format_header_content(ss, data);

        // 3. 主体内容
        if (is_empty_data(data)) {
            // [修改] 调用纯虚函数，由子类负责适配具体的 Config 接口
            ss << get_no_records_msg() << "\n";
        } else {
            format_extra_content(ss, data);
            format_project_tree_section(ss, data);
        }

        return ss.str();
    }

protected:
    std::shared_ptr<ConfigT> config_;

    // [修改] 注释掉未使用参数以消除警告
    virtual std::string validate_data(const ReportDataT& /*data*/) const { return ""; }
    
    virtual bool is_empty_data(const ReportDataT& data) const = 0;
    virtual int get_avg_days(const ReportDataT& data) const = 0;
    
    virtual void format_header_content(std::stringstream& ss, const ReportDataT& data) const = 0;
    
    // [修改] 注释掉未使用参数
    virtual void format_extra_content(std::stringstream& /*ss*/, const ReportDataT& /*data*/) const {}

    // [修改] 改为纯虚函数，移除导致编译错误的默认实现
    virtual std::string get_no_records_msg() const = 0;

    virtual void format_project_tree_section(std::stringstream& ss, const ReportDataT& data) const {
        ss << "\n## " << config_->get_project_breakdown_label() << "\n";
        ss << MarkdownFormatter::format_project_tree(
            data.project_tree,
            data.total_duration,
            get_avg_days(data)
        );
    }
};

#endif // REPORTS_SHARED_FORMATTERS_TEMPLATES_BASE_MD_FORMATTER_HPP_