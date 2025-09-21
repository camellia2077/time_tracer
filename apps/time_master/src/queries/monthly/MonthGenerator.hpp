// queries/monthly/MonthGenerator.hpp
#ifndef MONTHLY_REPORT_GENERATOR_HPP
#define MONTHLY_REPORT_GENERATOR_HPP
// ...
#include "common/AppConfig.hpp" // [新增]

class MonthGenerator {
public:
    explicit MonthGenerator(sqlite3* db, const AppConfig& config); // [修改]

    std::string generate_report(const std::string& year_month, ReportFormat format);

private:
    sqlite3* db_;
    const AppConfig& app_config_; // [修改]
};

#endif // MONTHLY_REPORT_GENERATOR_HPP