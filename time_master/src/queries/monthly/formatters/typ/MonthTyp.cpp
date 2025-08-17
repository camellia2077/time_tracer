
#include "MonthTyp.h"
#include <iomanip>
#include <format> // (新增) 引入 format 头文件

// --- 核心改动：引入所有需要的依赖 ---
#include "queries/shared/query_utils.h"
#include "queries/shared/breakdown/TreeFmtFactory.h"
#include "queries/shared/breakdown/ITreeFmt.h"
#include "common/common_utils.h"

#include "MonthTypStrings.h"

std::string MonthTyp::format_report(const MonthlyReportData& data, sqlite3* db) const {
    std::stringstream ss;
    
    // (修改) 使用 std::format，并使用 {0} 显式指定参数位置
    ss << std::format(R"(#set text(font: "{0}"))", MonthTypStrings::BodyFont) << "\n\n";

    if (data.year_month == "INVALID") {
        ss << MonthTypStrings::InvalidFormatError << "\n";
        return ss.str();
    }

    _display_summary(ss, data);

    if (data.actual_days == 0) {
        ss << MonthTypStrings::NoRecords << "\n";
        return ss.str();
    }

    _display_project_breakdown(ss, data, db);
    return ss.str();
}

void MonthTyp::_display_summary(std::stringstream& ss, const MonthlyReportData& data) const {
    // (修改) 使用 std::format 和位置参数动态构建标题
    std::string title = std::format(
        R"(#text(font: "{0}", size: {1}pt)[= {2} {3}-{4}])", // {0}{1}为样式, {2}{3}{4}为内容
        MonthTypStrings::TitleFont,                           // {0}
        MonthTypStrings::TitleFontSize,                       // {1}
        MonthTypStrings::TitlePrefix,                         // {2}
        data.year_month.substr(0, 4),                         // {3}
        data.year_month.substr(4, 2)                          // {4}
    );
    ss << title << "\n\n";

    if (data.actual_days > 0) {
        // (修改) 其余部分也使用 std::format 和位置参数
        ss << std::format("+ *{0}:* {1}\n", MonthTypStrings::ActualDaysLabel, data.actual_days);
        ss << std::format("+ *{0}:* {1}\n", MonthTypStrings::TotalTimeLabel, time_format_duration(data.total_duration, data.actual_days));
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