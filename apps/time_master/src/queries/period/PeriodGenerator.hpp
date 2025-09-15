// queries/period/PeriodGenerator.hpp
#ifndef PERIOD_REPORT_GENERATOR_HPP
#define PERIOD_REPORT_GENERATOR_HPP

#include <sqlite3.h>
#include <string>
#include "queries/shared/ReportFormat.hpp" // [新增] 引入报告格式的定义

class PeriodGenerator {
public:
    explicit PeriodGenerator(sqlite3* db, const std::string& period_typ_config_path);

    /**
     * @brief 为指定的周期生成格式化的报告。
     * @param days 要查询的天数。
     * @param format [修改] 需要生成的报告格式。
     * @return 包含格式化周期报告的字符串。
     */
    std::string generate_report(int days, ReportFormat format);

private:
    sqlite3* m_db;
    std::string m_period_typ_config_path; // [新增] 声明缺失的成员变量
};

#endif // PERIOD_REPORT_GENERATOR_HPP