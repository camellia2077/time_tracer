// queries/monthly/MonthGenerator.hpp
#ifndef MONTH_GENERATOR_HPP
#define MONTH_GENERATOR_HPP
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

#endif // MONTH_GENERATOR_HPP