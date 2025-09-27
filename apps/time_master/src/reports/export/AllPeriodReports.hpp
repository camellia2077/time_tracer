// reports/export/AllPeriodReports.hpp
#ifndef ALL_PERIOD_REPORTS_HPP
#define ALL_PERIOD_REPORTS_HPP

#include <sqlite3.h>
#include <vector>
#include "reports/shared/data/query_data_structs.hpp"
#include "reports/shared/types/ReportFormat.hpp"
#include "common/AppConfig.hpp" // [新增] 引入 AppConfig

class AllPeriodReports {
public:
    /**
     * @brief [修改] 构造函数现在接收一个指向 AppConfig 对象的常量引用。
     */
    explicit AllPeriodReports(sqlite3* db, const AppConfig& config);

    /**
     * @brief 为指定的周期列表生成所有报告。
     * @param days_list 一个包含要查询天数的整数向量 (例如 {7, 30})。
     * @param format 需要生成的报告格式。
     * @return 一个将天数映射到其对应格式化报告字符串的 map。
     */
    FormattedPeriodReports generate_reports(const std::vector<int>& days_list, ReportFormat format);

private:
    sqlite3* db_;
    const AppConfig& app_config_; // [修改] 保存 AppConfig 的引用
};

#endif // ALL_PERIOD_REPORTS_HPP