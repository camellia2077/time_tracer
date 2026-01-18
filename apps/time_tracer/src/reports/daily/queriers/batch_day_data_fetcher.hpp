// reports/daily/queriers/batch_day_data_fetcher.hpp
#ifndef REPORTS_DAILY_QUERIERS_BATCH_DAY_DATA_FETCHER_HPP_
#define REPORTS_DAILY_QUERIERS_BATCH_DAY_DATA_FETCHER_HPP_

#include <sqlite3.h>
#include <map>
#include <vector>
#include <string>
#include <tuple>
#include "reports/shared/model/daily_report_data.hpp"
#include "reports/shared/interfaces/i_project_info_provider.hpp"

// 结构体定义保持不变 ...
struct BatchDataResult {
    std::map<std::string, DailyReportData> data_map;
    std::vector<std::tuple<std::string, int, int>> date_order;
};

class BatchDayDataFetcher {
public:
    // [修改] 构造函数注入 provider
    explicit BatchDayDataFetcher(sqlite3* db, IProjectInfoProvider& provider);
    
    BatchDataResult fetch_all_data();

private:
    sqlite3* db_;
    IProjectInfoProvider& provider_; // [新增] 引用成员

    void fetch_days_metadata(BatchDataResult& result);
    void fetch_time_records(BatchDataResult& result);
};

#endif // REPORTS_DAILY_QUERIERS_BATCH_DAY_DATA_FETCHER_HPP_