// reports/period/formatters/tex/PeriodTex.cpp
#include "PeriodTex.hpp"
#include "PeriodTexUtils.hpp" // [新增] 引入新的辅助工具
#include "reports/shared/utils/format/TimeFormat.hpp"    
#include "reports/shared/formatters/latex/TexUtils.hpp"
#include "reports/shared/factories/GenericFormatterFactory.hpp"
#include "reports/period/formatters/tex/PeriodTexConfig.hpp"
#include "reports/shared/data/PeriodReportData.hpp"
#include <sstream>
#include <memory>

// 自我注册逻辑保持不变
namespace {
    struct PeriodTexRegister {
        PeriodTexRegister() {
            GenericFormatterFactory<PeriodReportData>::regist(ReportFormat::LaTeX, 
                [](const AppConfig& cfg) -> std::unique_ptr<IReportFormatter<PeriodReportData>> {
                    auto tex_config = std::make_shared<PeriodTexConfig>(cfg.period_tex_config_path);
                    return std::make_unique<PeriodTex>(tex_config);
                });
        }
    };
    const PeriodTexRegister registrar;
}

PeriodTex::PeriodTex(std::shared_ptr<PeriodTexConfig> config) : config_(config) {}

// [核心修改] format_report 方法现在只负责流程控制
std::string PeriodTex::format_report(const PeriodReportData& data) const {
    if (data.days_to_query <= 0) {
        return config_->get_invalid_days_message() + "\n";
    }

    std::stringstream ss;
    ss << TexUtils::get_tex_preamble(
        config_->get_main_font(), 
        config_->get_cjk_main_font(),
        config_->get_base_font_size(),
        config_->get_margin_in()
    );
    
    // 调用辅助函数来格式化摘要
    PeriodTexUtils::display_summary(ss, data, config_);

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