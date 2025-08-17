
#include "PeriodTyp.h"
#include <iomanip>
#include <format> // (新增) 引入 format 头文件

// --- 核心改动：引入所有需要的依赖 ---
#include "common/common_utils.h"
#include "queries/shared/query_utils.h"
#include "queries/shared/breakdown/TreeFmtFactory.h"
#include "queries/shared/breakdown/ITreeFmt.h"
#include "PeriodTypStrings.h" // 唯一且专属的配置文件

std::string PeriodTyp::format_report(const PeriodReportData& data, sqlite3* db) const {
    std::stringstream ss;
    
    // (修改) 使用 std::format，并使用 {0} 显式指定参数位置
    ss << std::format(R"(#set text(font: "{0}"))", PeriodTypStrings::ContentFont) << "\n\n";

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
    // (修改) 使用 std::format 和位置参数动态构建标题
    std::string title = std::format(
        R"(#text(font: "{0}", size: {1}pt)[= {2} {3} days ({4} to {5})])", // {0}{1}为样式, {2}-{5}为内容
        PeriodTypStrings::TitleFont,        // {0}
        PeriodTypStrings::TitleFontSize,    // {1}
        PeriodTypStrings::TitlePrefix,      // {2}
        data.days_to_query,                 // {3}
        data.start_date,                    // {4}
        data.end_date                       // {5}
    );
    ss << title << "\n\n";

    // (修改) 其余部分也使用 std::format 和位置参数
    if (data.actual_days > 0) {
        ss << std::format("+ *{0}:* {1}\n", PeriodTypStrings::TotalTimeLabel, time_format_duration(data.total_duration, data.actual_days));
        ss << std::format("+ *{0}:* {1}\n", PeriodTypStrings::ActualDaysLabel, data.actual_days);
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