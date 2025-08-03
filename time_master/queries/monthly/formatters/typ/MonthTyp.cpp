#include "common/pch.h"
#include "MonthTyp.h"
#include <iomanip>

#include "queries/shared/query_utils.h"
#include "queries/shared/breakdown/TreeFmtFactory.h"
#include "queries/shared/breakdown/ITreeFmt.h"
#include "common/common_utils.h"

std::string MonthTyp::format_report(const MonthlyReportData& data, sqlite3* db) const {
    std::stringstream ss;
    ss << "#set text(font: \"Noto Serif SC\")\n\n";
    if (data.year_month == "INVALID") {
        ss << "Invalid year_month format. Expected YYYYMM.\n";
        return ss.str();
    }

    _display_summary(ss, data);

    if (data.actual_days == 0) {
        ss << "No time records found for this month.\n";
        return ss.str();
    }

    _display_project_breakdown(ss, data, db);
    return ss.str();
}

void MonthTyp::_display_summary(std::stringstream& ss, const MonthlyReportData& data) const {
    ss << "= Monthly Summary for " << data.year_month.substr(0, 4) << "-" << data.year_month.substr(4, 2) << " \n\n";

    if (data.actual_days > 0) {
        ss << "+ *Actual Days with Records*: " << data.actual_days << "\n";
        ss << "+ *Total Time Recorded*: " << time_format_duration(data.total_duration, data.actual_days) << "\n";
    }
}

void MonthTyp::_display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) const {
    // 调用统一的工具函数来生成项目明细的 Typ 格式字符串
    ss << generate_project_breakdown(
        ReportFormat::Typ,     // 指定输出格式为 Typ
        db,                             // 传入数据库连接，用于获取父子类别映射
        data.records,               // 传入从月报数据中获取的时间记录
        data.total_duration,  // 传入总时长，用于计算各项百分比
        data.actual_days          // 对于月报，平均天数是该月实际有记录的天数
    );
}