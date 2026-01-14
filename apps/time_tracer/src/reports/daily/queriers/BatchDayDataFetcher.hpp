// reports/daily/queriers/BatchDayDataFetcher.hpp
#ifndef BATCH_DAY_DATA_FETCHER_HPP
#define BATCH_DAY_DATA_FETCHER_HPP

#include <sqlite3.h>
#include <map>
#include <vector>
#include <string>
#include <tuple>
#include "reports/shared/model/DailyReportData.hpp"
#include "reports/shared/interfaces/IProjectInfoProvider.hpp" // [新增]

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

#endif // BATCH_DAY_DATA_FETCHER_HPP