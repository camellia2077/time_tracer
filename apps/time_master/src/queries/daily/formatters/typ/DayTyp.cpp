// queries/daily/formatters/typ/DayTyp.cpp
#include "DayTyp.hpp"
#include "DayTypUtils.hpp" // [新增] 引入新的工具类
#include <format>
#include <string>

DayTyp::DayTyp(std::shared_ptr<DayTypConfig> config) : config_(config) {}

std::string DayTyp::format_report(const DailyReportData& data) const {
    std::stringstream ss;
    std::string spacing_str = std::to_string(config_->get_line_spacing_em()) + "em";
    ss << std::format(R"(#set text(font: "{}", size: {}pt, spacing: {}))", 
        config_->get_base_font(), 
        config_->get_base_font_size(),
        spacing_str) << "\n\n";

    // 使用工具类来构建报告的各个部分
    DayTypUtils::display_header(ss, data, config_);

    if (data.total_duration == 0) {
        ss << config_->get_no_records() << "\n";
        return ss.str();
    }
    
    DayTypUtils::display_statistics(ss, data, config_);
    DayTypUtils::display_detailed_activities(ss, data, config_);
    DayTypUtils::display_project_breakdown(ss, data, config_);
    
    return ss.str();
}