// reports/daily/formatters/tex/DayTex.cpp
#include "DayTex.hpp"
#include "DayTexUtils.hpp" // [修改] 引入新的辅助工具
#include "reports/shared/formatters/latex/TexUtils.hpp"
#include "reports/shared/factories/GenericFormatterFactory.hpp"
#include "reports/daily/formatters/tex/DayTexConfig.hpp"
#include "reports/shared/data/DailyReportData.hpp"
#include <sstream>
#include <memory>

// 自我注册逻辑保持不变
namespace {
    struct DayTexRegister {
        DayTexRegister() {
            GenericFormatterFactory<DailyReportData>::regist(ReportFormat::LaTeX,
                [](const AppConfig& cfg) -> std::unique_ptr<IReportFormatter<DailyReportData>> {
                    auto tex_config = std::make_shared<DayTexConfig>(cfg.day_tex_config_path);
                    return std::make_unique<DayTex>(tex_config);
                });
        }
    };
    const DayTexRegister registrar;
}

DayTex::DayTex(std::shared_ptr<DayTexConfig> config) : config_(config) {}

// [核心修改] format_report 方法现在只负责流程控制
std::string DayTex::format_report(const DailyReportData& data) const {
    std::stringstream ss;
    ss << TexUtils::get_tex_preamble(
        config_->get_main_font(),
        config_->get_cjk_main_font(),
        config_->get_base_font_size(),
        config_->get_margin_in(),
        config_->get_keyword_colors()
    );

    // 调用辅助函数来处理各个部分的格式化
    DayTexUtils::display_header(ss, data, config_);

    if (data.total_duration == 0) {
        ss << config_->get_no_records() << "\n";
    } else {
        DayTexUtils::display_statistics(ss, data, config_);
        DayTexUtils::display_detailed_activities(ss, data, config_);
        
        // 项目分解的逻辑比较简单，直接调用共享工具函数
        ss << TexUtils::format_project_tree(
            data.project_tree,
            data.total_duration,
            1, // 日报的 avg_days 总是 1
            config_->get_category_title_font_size(),
            config_->get_list_top_sep_pt(),
            config_->get_list_item_sep_ex()
        );
    }

    ss << TexUtils::get_tex_postfix();
    return ss.str();
}