#include "common/pch.h"
#include "PeriodTyp.h"
#include <iomanip>

#include "common/common_utils.h"
#include "queries/shared/query_utils.h"
#include "queries/shared/breakdown/TreeFmtFactory.h"
#include "queries/shared/breakdown/ITreeFmt.h"

std::string PeriodTyp::format_report(const PeriodReportData& data, sqlite3* db) const {
    std::stringstream ss;
    ss << "#set text(font: \"Noto Serif SC\")\n\n";
    if (data.days_to_query <= 0) {
        ss << "Number of days to query must be positive.\n";
        return ss.str();
    }

    _display_summary(ss, data);

    if (data.actual_days == 0) {
        ss << "No time records found in this period.\n";
        return ss.str();
    }
    
    _display_project_breakdown(ss, data, db);
    return ss.str();
}

void PeriodTyp::_display_summary(std::stringstream& ss, const PeriodReportData& data) const {
    ss << "= Period Report: Last " << data.days_to_query << " days ("
       << data.start_date << " to " << data.end_date << ") \n\n";

    if (data.actual_days > 0) {
        ss << "+ *Total Time Recorded*: " << time_format_duration(data.total_duration, data.actual_days) << "\n";
        ss << "+ *Actual Days with Records*: " << data.actual_days << "\n";
    }
}

void PeriodTyp::_display_project_breakdown(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) const {
    // 调用统一的工具函数来生成项目明细的 Typ 格式字符串
    ss << generate_project_breakdown(
        ReportFormat::Typ,     // 指定输出格式为 Typ
        db,                             // 传入数据库连接，用于获取父子类别映射
        data.records,               // 传入从月报数据中获取的时间记录
        data.total_duration,  // 传入总时长，用于计算各项百分比
        data.actual_days          // 对于月报，平均天数是该月实际有记录的天数
    );
}