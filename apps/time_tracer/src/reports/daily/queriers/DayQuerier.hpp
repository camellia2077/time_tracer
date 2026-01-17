// reports/daily/queriers/DayQuerier.hpp
#ifndef REPORTS_DAILY_QUERIERS_DAY_QUERIER_HPP_
#define REPORTS_DAILY_QUERIERS_DAY_QUERIER_HPP_

#include <sqlite3.h>
#include <string>
#include "reports/shared/model/DailyReportData.hpp" 
#include "reports/shared/queriers/BaseQuerier.hpp"

class DayQuerier : public BaseQuerier<DailyReportData, const std::string&> {
public:
    explicit DayQuerier(sqlite3* db, const std::string& date);

    DailyReportData fetch_data() override;

protected:
    std::string get_date_condition_sql() const override;
    void bind_sql_parameters(sqlite3_stmt* stmt) const override;
    void _prepare_data(DailyReportData& data) const override;

private:
    void _fetch_metadata(DailyReportData& data);
    void _fetch_detailed_records(DailyReportData& data);
    // --- [核心修改] 方法重命名，功能扩展 ---
    void _fetch_generated_stats(DailyReportData& data);
};

#endif // REPORTS_DAILY_QUERIERS_DAY_QUERIER_HPP_