// reports/shared/formatters/templates/BaseTexFormatter.hpp
#ifndef BASE_TEX_FORMATTER_HPP
#define BASE_TEX_FORMATTER_HPP

#include <string>
#include <sstream>
#include <memory>
#include <map>
#include "reports/shared/interfaces/IReportFormatter.hpp"
#include "reports/shared/formatters/latex/TexUtils.hpp"

/**
 * @class BaseTexFormatter
 * @brief LaTeX 报告格式化器的通用模板基类。
 */
template <typename ReportDataT, typename ConfigT>
class BaseTexFormatter : public IReportFormatter<ReportDataT> {
public:
    explicit BaseTexFormatter(std::shared_ptr<ConfigT> config) : config_(config) {}

    std::string format_report(const ReportDataT& data) const override {
        // 1. 数据有效性检查
        if (std::string err = validate_data(data); !err.empty()) {
            return err;
        }

        std::stringstream ss;
        
        // 2. Preamble
        ss << generate_preamble();

        // 3. 头部 / 摘要
        format_header_content(ss, data);

        // 4. 主体内容
        if (is_empty_data(data)) {
            // [修改] 统一使用 get_no_records_msg() 钩子
            ss << get_no_records_msg() << "\n";
        } else {
            // 钩子：用于 Daily 报告插入统计信息和详细活动记录
            format_extra_content(ss, data);
            
            // 项目树部分
            format_project_tree_section(ss, data);
        }

        // 5. Postfix
        ss << generate_postfix();
        return ss.str();
    }

protected:
    std::shared_ptr<ConfigT> config_;

    // [修改] 注释掉未使用参数
    virtual std::string validate_data(const ReportDataT& /*data*/) const { return ""; }

    virtual bool is_empty_data(const ReportDataT& data) const = 0;
    virtual int get_avg_days(const ReportDataT& data) const = 0;

    // [新增] 纯虚函数，强制子类适配 Config 接口
    virtual std::string get_no_records_msg() const = 0;

    virtual void format_header_content(std::stringstream& ss, const ReportDataT& data) const = 0;

    // [修改] 注释掉未使用参数
    virtual void format_extra_content(std::stringstream& /*ss*/, const ReportDataT& /*data*/) const {}

    virtual std::map<std::string, std::string> get_keyword_colors() const { return {}; }

    virtual std::string generate_preamble() const {
        return TexUtils::get_tex_preamble(
            config_->get_main_font(),
            config_->get_cjk_main_font(),
            config_->get_base_font_size(),
            config_->get_margin_in(),
            get_keyword_colors()
        );
    }

    virtual void format_project_tree_section(std::stringstream& ss, const ReportDataT& data) const {
        int title_size = config_->get_category_title_font_size();
        ss << "{";
        ss << "\\fontsize{" << title_size << "}{" << title_size * 1.2 << "}\\selectfont";
        ss << "\\section*{" << TexUtils::escape_latex(config_->get_project_breakdown_label()) << "}";
        ss << "}\n\n";

        ss << TexUtils::format_project_tree(
            data.project_tree,
            data.total_duration,
            get_avg_days(data),
            config_->get_category_title_font_size(),
            config_->get_list_top_sep_pt(),
            config_->get_list_item_sep_ex()
        );
    }

    virtual std::string generate_postfix() const {
        return TexUtils::get_tex_postfix();
    }
};

#endif // BASE_TEX_FORMATTER_HPP