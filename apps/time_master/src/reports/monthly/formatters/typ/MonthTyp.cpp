// reports/monthly/formatters/typ/MonthTyp.cpp
#include "MonthTyp.hpp"
#include <iomanip>
#include <format>
#include <vector>
#include <algorithm>
#include "reports/shared/utils/format/TimeFormat.hpp"
#include "reports/shared/utils/format/TypUtils.hpp"      
#include "reports/shared/factories/GenericFormatterFactory.hpp" 
#include "reports/monthly/formatters/typ/MonthTypConfig.hpp"   
#include "reports/shared/data/MonthlyReportData.hpp"       

// [新增] 自我注册逻辑
namespace {
    struct MonthTypRegister {
        MonthTypRegister() {
            GenericFormatterFactory<MonthlyReportData>::regist(ReportFormat::Typ, 
                [](const AppConfig& cfg) -> std::unique_ptr<IReportFormatter<MonthlyReportData>> {
                    auto typ_config = std::make_shared<MonthTypConfig>(cfg.month_typ_config_path);
                    return std::make_unique<MonthTyp>(typ_config);
                });
        }
    };
    const MonthTypRegister registrar;
}


MonthTyp::MonthTyp(std::shared_ptr<MonthTypConfig> config) : config_(config) {} 

std::string MonthTyp::format_report(const MonthlyReportData& data) const {
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

    if (data.year_month == "INVALID") {
        ss << config_->get_invalid_format_message() << "\n";
        return ss.str();
    }

    _display_summary(ss, data);

    if (data.actual_days == 0) {
        ss << config_->get_no_records_message() << "\n";
        return ss.str();
    }

    _display_project_breakdown(ss, data);
    return ss.str();
}

void MonthTyp::_display_summary(std::stringstream& ss, const MonthlyReportData& data) const {
    std::string title = std::format(
        R"(#text(font: "{}", size: {}pt)[= {} {}-{}])",
        config_->get_title_font(),
        config_->get_report_title_font_size(),
        config_->get_report_title(),
        data.year_month.substr(0, 4),
        data.year_month.substr(4, 2)
    );
    ss << title << "\n\n";

    if (data.actual_days > 0) {
        ss << std::format("+ *{}:* {}\n", config_->get_actual_days_label(), data.actual_days);
        ss << std::format("+ *{}:* {}\n", config_->get_total_time_label(), time_format_duration(data.total_duration, data.actual_days));
    }
}

void MonthTyp::_display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data) const {
    // [核心修改] 调用共享的 TypUtils 来格式化项目树
    ss << TypUtils::format_project_tree(
        data.project_tree,
        data.total_duration,
        data.actual_days,
        config_->get_category_title_font(),
        config_->get_category_title_font_size()
    );
}