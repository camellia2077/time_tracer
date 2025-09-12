// queries/daily/formatters/tex/DayTex.cpp
#include "DayTex.hpp"
#include "DayTexConfig.hpp"
#include "DayTexStrings.hpp"

#include <iomanip>
#include <string>
#include <sstream>

#include "common/common_utils.hpp"

#include "queries/shared/utils/query_utils.hpp"
#include "queries/shared/utils/BoolToString.hpp"
#include "queries/shared/factories/TreeFmtFactory.hpp"
#include "queries/shared/Interface/ITreeFmt.hpp"
#include "queries/shared/data/DailyReportData.hpp"
#include "queries/shared/utils/TimeFormat.hpp" // [新增] 引入 TimeFormat.hpp

// Local helper function to escape special TeX characters.
namespace {
    std::string escape_tex_local(const std::string& s) {
        std::string escaped;
        escaped.reserve(s.length());
        for (char c : s) {
            if (c == '&' || c == '%' || c == '$' || c == '#' || c == '_' || c == '{' || c == '}') {
                escaped += '\\';
            }
            escaped += c;
        }
        return escaped;
    }
}

std::string DayTex::format_report(const DailyReportData& data, sqlite3* db) const {
    return format_report_template(data, db);
}

void DayTex::format_content(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const {
    _display_header(ss, data);

    if (data.total_duration == 0) {
        ss << DayTexConfig::NoRecordsMessage << "\n";
    } else {
        // [修改] 调用新增的函数来显示统计和活动详情
        _display_statistics(ss, data);
        _display_detailed_activities(ss, data);
        _display_project_breakdown(ss, data, db);
    }
}

void DayTex::_display_header(std::stringstream& ss, const DailyReportData& data) const {
    ss << "\\section*{" << DayTexConfig::ReportTitle << " " << escape_tex_local(data.date) << "}\n\n";
    
    ss << "\\begin{itemize}\n";
    ss << "    \\item \\textbf{" << DayTexConfig::DateLabel      << "}: " << escape_tex_local(data.date) << "\n";
    ss << "    \\item \\textbf{" << DayTexConfig::TotalTimeLabel << "}: " << escape_tex_local(time_format_duration(data.total_duration)) << "\n";
    ss << "    \\item \\textbf{" << DayTexConfig::StatusLabel    << "}: " << escape_tex_local(bool_to_string(data.metadata.status)) << "\n";
    ss << "    \\item \\textbf{" << DayTexConfig::SleepLabel     << "}: " << escape_tex_local(bool_to_string(data.metadata.sleep)) << "\n";
    // --- [核心修改] 新增 exercise 状态的显示 ---
    ss << "    \\item \\textbf{" << DayTexConfig::ExerciseLabel  << "}: " << escape_tex_local(bool_to_string(data.metadata.exercise)) << "\n";
    ss << "    \\item \\textbf{" << DayTexConfig::GetupTimeLabel << "}: " << escape_tex_local(data.metadata.getup_time) << "\n";
    ss << "    \\item \\textbf{" << DayTexConfig::RemarkLabel    << "}: " << escape_tex_local(data.metadata.remark) << "\n";
    ss << "\\end{itemize}\n\n";
}

void DayTex::_display_project_breakdown(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const {
    ss << generate_project_breakdown(
        ReportFormat::LaTeX, 
        db, 
        data.records, 
        data.total_duration, 
        1 
    );
}

// [新增] 显示统计信息
void DayTex::_display_statistics(std::stringstream& ss, const DailyReportData& data) const {
    ss << "\\subsection*{" << DayTexConfig::StatisticsLabel << "}\n\n";
    ss << "\\begin{itemize}\n";
    ss << "    \\item \\textbf{" << DayTexConfig::SleepTimeLabel << "}: "
       << escape_tex_local(time_format_duration_hm(data.sleep_time)) << "\n";
    ss << "\\end{itemize}\n\n";
}

// [新增] 显示详细活动
void DayTex::_display_detailed_activities(std::stringstream& ss, const DailyReportData& data) const {
    if (data.detailed_records.empty()) {
        return;
    }

    ss << "\\subsection*{" << DayTexConfig::AllActivitiesLabel << "}\n\n";
    ss << "\\begin{itemize}\n";

    for (const auto& record : data.detailed_records) {
        std::string base_string = escape_tex_local(record.start_time) + " - " +
                                  escape_tex_local(record.end_time) + " (" +
                                  escape_tex_local(time_format_duration_hm(record.duration_seconds)) +
                                  "): " + escape_tex_local(record.project_path);
        
        std::string colorized_string = base_string;

        // Check for a matching keyword and apply color if found
        for (const auto& pair : DayTexStrings::KeywordColors) {
            if (record.project_path.find(pair.first) != std::string::npos) {
                colorized_string = "\\textcolor{" + pair.second + "}{" + base_string + "}";
                break; // Exit the loop once a color is applied
            }
        }
        
        ss << "    \\item " << colorized_string << "\n";

        if (record.activityRemark.has_value()) {
            ss << "    \\begin{itemize}\n";
            ss << "        \\item \\textbf{" << DayTexConfig::ActivityRemarkLabel << "}: " 
               << escape_tex_local(record.activityRemark.value()) << "\n";
            ss << "    \\end{itemize}\n";
        }
    }
    ss << "\\end{itemize}\n\n";
}