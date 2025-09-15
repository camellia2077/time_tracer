// queries/export/AllPeriodReports.hpp
#ifndef ALL_PERIOD_REPORTS_GENERATOR_HPP
#define ALL_PERIOD_REPORTS_GENERATOR_HPP

#include <sqlite3.h>
#include <vector>
#include "queries/shared/data/query_data_structs.hpp"
#include "queries/shared/ReportFormat.hpp"

class AllPeriodReports {
public:
    explicit AllPeriodReports(sqlite3* db, const std::string& period_typ_config_path, const std::string& period_md_config_path);

    FormattedPeriodReports generate_reports(const std::vector<int>& days_list, ReportFormat format);

private:
    sqlite3* m_db;
    std::string m_period_typ_config_path;
    std::string m_period_md_config_path;
};

#endif // ALL_PERIOD_REPORTS_GENERATOR_HPP