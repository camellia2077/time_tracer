// queries/monthly/formatters/typ/MonthTyp.cpp
#include "MonthTyp.hpp"
#include <iomanip>
#include <format>
#include "queries/shared/utils/query_utils.hpp"
#include "queries/shared/factories/TreeFmtFactory.hpp"

// [删除] #include "MonthTypStrings.hpp"

MonthTyp::MonthTyp(std::shared_ptr<MonthTypConfig> config) : config_(config) {} // [新增] 构造函数实现

std::string MonthTyp::format_report(const MonthlyReportData& data, sqlite3* db) const {
    std::stringstream ss;
    
    ss << std::format(R"(#set text(font: "{0}"))", config_->get_body_font()) << "\n\n";

    if (data.year_month == "INVALID") {
        ss << config_->get_invalid_format_error() << "\n";
        return ss.str();
    }

    _display_summary(ss, data);

    if (data.actual_days == 0) {
        ss << config_->get_no_records() << "\n";
        return ss.str();
    }

    _display_project_breakdown(ss, data, db);
    return ss.str();
}

void MonthTyp::_display_summary(std::stringstream& ss, const MonthlyReportData& data) const {
    std::string title = std::format(
        R"(#text(font: "{0}", size: {1}pt)[= {2} {3}-{4}])",
        config_->get_title_font(),
        config_->get_title_font_size(),
        config_->get_title_prefix(),
        data.year_month.substr(0, 4),
        data.year_month.substr(4, 2)
    );
    ss << title << "\n\n";

    if (data.actual_days > 0) {
        ss << std::format("+ *{0}:* {1}\n", config_->get_actual_days_label(), data.actual_days);
        ss << std::format("+ *{0}:* {1}\n", config_->get_total_time_label(), time_format_duration(data.total_duration, data.actual_days));
    }
}

void MonthTyp::_display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) const {
    ss << generate_project_breakdown(
        ReportFormat::Typ,
        db,
        data.records,
        data.total_duration,
        data.actual_days
    );
}