// queries/period/PeriodGenerator.hpp
#ifndef PERIOD_REPORT_GENERATOR_HPP
#define PERIOD_REPORT_GENERATOR_HPP

#include <sqlite3.h>
#include <string>
#include "queries/shared/ReportFormat.hpp"

class PeriodGenerator {
public:
    explicit PeriodGenerator(sqlite3* db, const std::string& period_typ_config_path, const std::string& period_md_config_path);

    std::string generate_report(int days, ReportFormat format);

private:
    sqlite3* m_db;
    std::string m_period_typ_config_path;
    std::string m_period_md_config_path;
};

#endif // PERIOD_REPORT_GENERATOR_HPP