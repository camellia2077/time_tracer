// queries/monthly/MonthGenerator.hpp
#ifndef MONTHLY_REPORT_GENERATOR_HPP
#define MONTHLY_REPORT_GENERATOR_HPP

#include <sqlite3.h>
#include <string>
#include "queries/shared/ReportFormat.hpp"

class MonthGenerator {
public:
    explicit MonthGenerator(sqlite3* db, const std::string& month_typ_config_path, const std::string& month_md_config_path);

    std::string generate_report(const std::string& year_month, ReportFormat format);

private:
    sqlite3* m_db;
    std::string m_month_typ_config_path;
    std::string m_month_md_config_path;
};

#endif // MONTHLY_REPORT_GENERATOR_HPP