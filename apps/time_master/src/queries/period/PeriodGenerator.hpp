#ifndef PERIOD_REPORT_GENERATOR_HPP
#define PERIOD_REPORT_GENERATOR_HPP

#include <sqlite3.h>
#include <string>
#include "queries/shared/ReportFormat.hpp"
#include "common/AppConfig.hpp"

class PeriodGenerator {
public:
    explicit PeriodGenerator(sqlite3* db, const AppConfig& config);

    std::string generate_report(int days, ReportFormat format);

private:
    sqlite3* m_db;
    const AppConfig& app_config_;
};

#endif // PERIOD_REPORT_GENERATOR_HPP