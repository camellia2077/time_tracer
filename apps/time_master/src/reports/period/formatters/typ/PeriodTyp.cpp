// reports/period/formatters/typ/PeriodTyp.cpp
#include "PeriodTyp.hpp"
#include <iomanip>
#include <format>
#include <vector>
#include <algorithm>
#include "reports/shared/utils/format/TimeFormat.hpp"
#include "reports/shared/utils/format/TypUtils.hpp"      // [新增]
#include "reports/shared/factories/GenericFormatterFactory.hpp" // [新增]
#include "reports/period/formatters/typ/PeriodTypConfig.hpp"    // [新增]
#include "reports/shared/data/PeriodReportData.hpp"            // [新增]

// [新增] 自我注册逻辑
namespace {
    struct PeriodTypRegister {
        PeriodTypRegister() {
            GenericFormatterFactory<PeriodReportData>::regist(ReportFormat::Typ, 
                [](const AppConfig& cfg) -> std::unique_ptr<IReportFormatter<PeriodReportData>> {
                    auto typ_config = std::make_shared<PeriodTypConfig>(cfg.period_typ_config_path);
                    return std::make_unique<PeriodTyp>(typ_config);
                });
        }
    };
    const PeriodTypRegister registrar;
}


PeriodTyp::PeriodTyp(std::shared_ptr<PeriodTypConfig> config) : config_(config) {}

std::string PeriodTyp::format_report(const PeriodReportData& data) const {
    std::stringstream ss;

    ss << std::format(R"(#set page(margin: (top: {}cm, bottom: {}cm, left: {}cm, right: {}cm)))",
        config_->get_margin_top_cm(),
        config_->get_margin_bottom_cm(),
        config_->get_margin_left_cm(),
        config_->get_margin_right_cm()
    ) << "\n";
    
    std::string spacing_str = std::to_string(config_->get_line_spacing_em()) + "em";
    ss << std::format(R"(#set text(font: "{}", size: {}pt, spacing: {}))", 
        config_->get_base_font(),
        config_->get_base_font_size(),
        spacing_str
    ) << "\n\n";

    if (data.days_to_query <= 0) {
        ss << config_->get_invalid_days_message() << "\n";
        return ss.str();
    }

    _display_summary(ss, data);

    if (data.actual_days == 0) {
        ss << config_->get_no_records() << "\n";
        return ss.str();
    }
    
    _display_project_breakdown(ss, data);
    return ss.str();
}

void PeriodTyp::_display_summary(std::stringstream& ss, const PeriodReportData& data) const {
    std::string title = std::format(
        R"(#text(font: "{}", size: {}pt)[= {} {} {} ({} {} {})])",
        config_->get_title_font(),
        config_->get_report_title_font_size(),
        config_->get_report_title_prefix(),
        data.days_to_query,
        config_->get_report_title_days(),
        data.start_date,
        config_->get_report_title_date_separator(),
        data.end_date
    );
    ss << title << "\n\n";

    if (data.actual_days > 0) {
        ss << std::format("+ *{}:* {}\n", config_->get_total_time_label(), time_format_duration(data.total_duration, data.actual_days));
        ss << std::format("+ *{}:* {}\n", config_->get_actual_days_label(), data.actual_days);
    }
}

void PeriodTyp::_display_project_breakdown(std::stringstream& ss, const PeriodReportData& data) const {
    // [核心修改] 调用共享的 TypUtils 来格式化项目树
    ss << TypUtils::format_project_tree(
        data.project_tree,
        data.total_duration,
        data.actual_days,
        config_->get_category_title_font(),
        config_->get_category_title_font_size()
    );
}