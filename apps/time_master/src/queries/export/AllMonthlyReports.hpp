// queries/export/AllMonthlyReports.hpp
#ifndef ALL_MONTHLY_REPORTS_HPP
#define ALL_MONTHLY_REPORTS_HPP

#include <sqlite3.h>
#include <map>
#include <string>
#include "queries/shared/data/query_data_structs.hpp"
#include "queries/shared/types/ReportFormat.hpp"
#include "common/AppConfig.hpp"

class AllMonthlyReports {
public:
    explicit AllMonthlyReports(sqlite3* db, const AppConfig& config);

    FormattedMonthlyReports generate_reports(ReportFormat format);

private:
    sqlite3* db_;
    const AppConfig& app_config_;
};

#endif // ALL_MONTHLY_REPORTS_HPP