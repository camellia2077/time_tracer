#include "common/pch.h"
#include "PeriodTyp.h"
#include <iomanip>

// --- 核心改动：引入所有需要的依赖 ---
#include "common/common_utils.h"
#include "queries/shared/query_utils.h"
#include "queries/shared/breakdown/TreeFmtFactory.h"
#include "queries/shared/breakdown/ITreeFmt.h"
#include "PeriodTypStrings.h" // 唯一且专属的配置文件

std::string PeriodTyp::format_report(const PeriodReportData& data, sqlite3* db) const {
    std::stringstream ss;
    
    // 1. 根据配置，将文档的全局基础字体设置为“正文字体”
    ss << "#set text(font: \"" << PeriodTypStrings::ContentFont << "\")\n\n";

    if (data.days_to_query <= 0) {
        ss << PeriodTypStrings::PositiveDaysError << "\n";
        return ss.str();
    }

    _display_summary(ss, data);

    if (data.actual_days == 0) {
        ss << PeriodTypStrings::NoRecords << "\n";
        return ss.str();
    }
    
    _display_project_breakdown(ss, data, db);
    return ss.str();
}

void PeriodTyp::_display_summary(std::stringstream& ss, const PeriodReportData& data) const {
    // 2. 对于大标题，局部、显式地使用“标题字体”
    ss << "#text(font: \"" << PeriodTypStrings::TitleFont << "\")[= " 
       << PeriodTypStrings::TitlePrefix << " " << data.days_to_query << " days ("
       << data.start_date << " to " << data.end_date << ")]\n\n";

    // 3. 其他信息将自动使用全局设置的“正文字体”
    if (data.actual_days > 0) {
        ss << "+ *" << PeriodTypStrings::TotalTimeLabel << "*: " << time_format_duration(data.total_duration, data.actual_days) << "\n";
        ss << "+ *" << PeriodTypStrings::ActualDaysLabel << "*: " << data.actual_days << "\n";
    }
}

void PeriodTyp::_display_project_breakdown(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) const {
    ss << generate_project_breakdown(
        ReportFormat::Typ,
        db,
        data.records,
        data.total_duration,
        data.actual_days
    );
}