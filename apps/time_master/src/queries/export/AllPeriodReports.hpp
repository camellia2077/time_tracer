// queries/export/AllPeriodReports.hpp
#ifndef ALL_PERIOD_REPORTS_GENERATOR_HPP
#define ALL_PERIOD_REPORTS_GENERATOR_HPP

#include <sqlite3.h>
#include <vector>
#include "queries/shared/data/query_data_structs.hpp"
#include "queries/shared/ReportFormat.hpp" // 引入报告格式的定义

/**
 * @class AllPeriodReports
 * @brief 生成所有指定天数的周期报告。
 *
 * 该类接收一个包含多个天数的列表，为每个天数生成一个周期报告，
 * 并返回一个包含所有报告内容的 map。
 */
class AllPeriodReports {
public:
    explicit AllPeriodReports(sqlite3* db, const std::string& period_typ_config_path);

    /**
     * @brief 根据提供的天数列表生成所有周期报告。
     * @param days_list 一个包含多个天数的 vector，例如 {7, 30, 90}。
     * @param format [修改] 需要生成的报告格式。
     * @return 一个 map，键是天数，值是格式化后的报告字符串。
     */
    FormattedPeriodReports generate_reports(const std::vector<int>& days_list, ReportFormat format);

private:
    sqlite3* m_db;
    std::string m_period_typ_config_path;
};

#endif // ALL_PERIOD_REPORTS_GENERATOR_HPP