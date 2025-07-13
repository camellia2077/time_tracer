#ifndef PERIOD_REPORT_GENERATOR_H
#define PERIOD_REPORT_GENERATOR_H

#include <sqlite3.h>
#include <string>

class PeriodReportGenerator {
public:
    explicit PeriodReportGenerator(sqlite3* db);

    std::string generate_report(int days);

private:
    sqlite3* m_db;
};

#endif // PERIOD_REPORT_GENERATOR_H