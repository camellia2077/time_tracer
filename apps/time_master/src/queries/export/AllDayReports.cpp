// queries/export/AllDayReports.cpp

#include "AllDayReports.hpp"
#include "queries/daily/DayQuerier.hpp"
#include <stdexcept>
#include <vector>
#include <memory>

#include "queries/shared/factories/FmtFactory.hpp"
#include "queries/daily/formatters/md/DayMd.hpp"
#include "queries/daily/formatters/md/DayMdConfig.hpp"
#include "queries/daily/formatters/tex/DayTex.hpp"
#include "queries/daily/formatters/tex/DayTexConfig.hpp" // [新增]
#include "queries/daily/formatters/typ/DayTyp.hpp"
#include "queries/daily/formatters/typ/DayTypConfig.hpp"

AllDayReports::AllDayReports(sqlite3* db, const std::string& day_typ_config_path, const std::string& day_md_config_path, const std::string& day_tex_config_path)
    : m_db(db), m_day_typ_config_path(day_typ_config_path), m_day_md_config_path(day_md_config_path), m_day_tex_config_path(day_tex_config_path) {
    if (m_db == nullptr) {
        throw std::invalid_argument("Database connection cannot be null.");
    }
}

FormattedGroupedReports AllDayReports::generate_all_reports(ReportFormat format) {
    FormattedGroupedReports grouped_reports;
    sqlite3_stmt* stmt;

    const char* sql = "SELECT date, year, month FROM days ORDER BY date ASC;";

    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement to fetch all dates.");
    }

    std::unique_ptr<IReportFormatter<DailyReportData>> formatter;
    switch (format) {
        case ReportFormat::Typ: {
            auto config = std::make_shared<DayTypConfig>(m_day_typ_config_path);
            formatter = std::make_unique<DayTyp>(config);
            break;
        }
        case ReportFormat::Markdown: {
            auto config = std::make_shared<DayMdConfig>(m_day_md_config_path);
            formatter = std::make_unique<DayMd>(config);
            break;
        }
        case ReportFormat::LaTeX: {
            // [修改] 为 DayTex 加载并传递配置
            auto config = std::make_shared<DayTexConfig>(m_day_tex_config_path);
            formatter = std::make_unique<DayTex>(config);
            break;
        }
    }


    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* date_cstr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (!date_cstr) continue;

        std::string date(date_cstr);
        int year = sqlite3_column_int(stmt, 1);
        int month = sqlite3_column_int(stmt, 2);

        DayQuerier querier(m_db, date);
        DailyReportData report_data = querier.fetch_data();

        std::string formatted_report = formatter->format_report(report_data, m_db);
        grouped_reports[year][month].push_back({date, formatted_report});
    }

    sqlite3_finalize(stmt);
    return grouped_reports;
}