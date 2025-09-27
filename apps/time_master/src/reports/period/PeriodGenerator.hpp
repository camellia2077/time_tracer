// reports/period/PeriodGenerator.hpp
#ifndef PERIOD_GENERATOR_HPP
#define PERIOD_GENERATOR_HPP

#include <sqlite3.h>
#include <string>
#include "reports/shared/types/ReportFormat.hpp"
#include "common/AppConfig.hpp"

class PeriodGenerator {
public:
    explicit PeriodGenerator(sqlite3* db, const AppConfig& config);

    std::string generate_report(int days, ReportFormat format);

private:
    sqlite3* db_;
    const AppConfig& app_config_;
};

#endif // PERIOD_GENERATOR_HPP