#include "common/pch.h"
#include "MonthTyp.h"
#include <iomanip>

// --- 核心改动：引入所有需要的依赖 ---
#include "queries/shared/query_utils.h"
#include "queries/shared/breakdown/TreeFmtFactory.h"
#include "queries/shared/breakdown/ITreeFmt.h"
#include "common/common_utils.h"

#include "MonthTypStrings.h"

std::string MonthTyp::format_report(const MonthlyReportData& data, sqlite3* db) const {
    std::stringstream ss;
    
    // 2. 根据专属配置文件中的字体设置，动态生成 Preamble
    ss << "#set text(font: \"" << MonthTypStrings::BodyFont << "\")\n\n";

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
    // 3. 动态构建标题，未来可以轻松地为标题应用不同的字体
    // 例如: ss << "#text(font: \"" << MonthTypStrings::TitleFont << "\")[= " ... "]\n\n";
    ss << "= " << MonthTypStrings::TitlePrefix << " " << data.year_month.substr(0, 4) << "-" << data.year_month.substr(4, 2) << " \n\n";

    if (data.actual_days > 0) {
        ss << "+ *" << MonthTypStrings::ActualDaysLabel << "*: " << data.actual_days << "\n";
        ss << "+ *" << MonthTypStrings::TotalTimeLabel << "*: " << time_format_duration(data.total_duration, data.actual_days) << "\n";
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