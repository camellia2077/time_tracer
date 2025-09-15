// queries/export/AllMonthlyReports.hpp
#ifndef ALL_MONTHLY_REPORTS_GENERATOR_HPP
#define ALL_MONTHLY_REPORTS_GENERATOR_HPP

#include <sqlite3.h>
#include <map>
#include <string>
#include "queries/shared/data/query_data_structs.hpp"
#include "queries/shared/ReportFormat.hpp"

class AllMonthlyReports {
public:
    explicit AllMonthlyReports(sqlite3* db, const std::string& month_typ_config_path, const std::string& month_md_config_path);

    FormattedMonthlyReports generate_reports(ReportFormat format);

private:
    sqlite3* m_db;
    std::string m_month_typ_config_path;
    std::string m_month_md_config_path;
};

#endif // ALL_MONTHLY_REPORTS_GENERATOR_HPP