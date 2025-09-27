// reports/monthly/formatters/tex/MonthTex.cpp
#include "MonthTex.hpp"
#include "MonthTexUtils.hpp" // [新增] 引入新的辅助工具
#include "reports/shared/utils/format/TimeFormat.hpp"     
#include "reports/shared/formatters/latex/TexUtils.hpp"
#include "reports/shared/factories/GenericFormatterFactory.hpp"
#include "reports/monthly/formatters/tex/MonthTexConfig.hpp"
#include "reports/shared/data/MonthlyReportData.hpp"
#include <sstream>
#include <memory>

// 自我注册逻辑保持不变
namespace {
    struct MonthTexRegister {
        MonthTexRegister() {
            GenericFormatterFactory<MonthlyReportData>::regist(ReportFormat::LaTeX, 
                [](const AppConfig& cfg) -> std::unique_ptr<IReportFormatter<MonthlyReportData>> {
                    auto tex_config = std::make_shared<MonthTexConfig>(cfg.month_tex_config_path);
                    return std::make_unique<MonthTex>(tex_config);
                });
        }
    };
    const MonthTexRegister registrar;
}

MonthTex::MonthTex(std::shared_ptr<MonthTexConfig> config) : config_(config) {}

// [核心修改] format_report 方法现在只负责流程控制
std::string MonthTex::format_report(const MonthlyReportData& data) const {
    if (data.year_month == "INVALID") {
        return config_->get_invalid_format_message() + "\n";
    }

    std::stringstream ss;
    ss << TexUtils::get_tex_preamble(
        config_->get_main_font(), 
        config_->get_cjk_main_font(),
        config_->get_base_font_size(),
        config_->get_margin_in()
    );

    // 调用辅助函数来格式化摘要
    MonthTexUtils::display_summary(ss, data, config_);

    if (data.actual_days == 0) {
        ss << config_->get_no_records_message() << "\n";
    } else {
        // 直接调用共享的 TexUtils 来格式化项目分解
        ss << TexUtils::format_project_tree(
            data.project_tree,
            data.total_duration,
            data.actual_days,
            config_->get_category_title_font_size(),
            config_->get_list_top_sep_pt(),
            config_->get_list_item_sep_ex()
        );
    }
    
    ss << TexUtils::get_tex_postfix();
    return ss.str();
}