#include "common/pch.h"
#include "DayTyp.h"
#include <iomanip>

// --- 核心改动：引入所有需要的依赖 ---
#include "common/common_utils.h"
#include "queries/shared/query_utils.h"
#include "queries/shared/DailyReportData.h"
#include "DayTypStrings.h" // 唯一且专属的配置文件

std::string DayTyp::format_report(const DailyReportData& data, sqlite3* db) const {
    std::stringstream ss;
    
    // 1. (修正) 根据配置，将文档的全局基础字体设置为“正文字体”
    ss << "#set text(font: \"" << DayTypStrings::ContentFont << "\")\n\n";

    _display_header(ss, data);

    if (data.total_duration == 0) {
        ss << DayTypStrings::NoRecords << "\n";
        return ss.str();
    }

    _display_project_breakdown(ss, data, db);
    return ss.str();
}

void DayTyp::_display_header(std::stringstream& ss, const DailyReportData& data) const {
    // 2. (修正) 对于大标题，局部、显式地使用“标题字体”
    ss << "#text(font: \"" << DayTypStrings::TitleFont << "\")[= " << DayTypStrings::TitlePrefix << " " << data.date << "]\n\n";
    
    // 其他头部信息将自动使用全局设置的“正文字体”
    ss << "+ *" << DayTypStrings::DateLabel << "*: " << data.date << "\n";
    ss << "+ *" << DayTypStrings::TotalTimeLabel << "*: " << time_format_duration(data.total_duration) << "\n";
    ss << "+ *" << DayTypStrings::StatusLabel << "*: " << data.metadata.status << "\n";
    ss << "+ *" << DayTypStrings::GetupTimeLabel << "*: " << data.metadata.getup_time << "\n";
    ss << "+ *" << DayTypStrings::RemarkLabel << "*:" << data.metadata.remark << "\n";
}

void DayTyp::_display_project_breakdown(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const {
    ss << generate_project_breakdown(
        ReportFormat::Typ, 
        db, 
        data.records, 
        data.total_duration, 
        1
    );
}