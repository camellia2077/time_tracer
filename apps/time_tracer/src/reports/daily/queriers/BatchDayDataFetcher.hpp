// reports/daily/queriers/BatchDayDataFetcher.hpp
#ifndef BATCH_DAY_DATA_FETCHER_HPP
#define BATCH_DAY_DATA_FETCHER_HPP

#include <sqlite3.h>
#include <map>
#include <vector>
#include <string>
#include <tuple>
#include "reports/shared/model/DailyReportData.hpp"

// 定义返回的数据包，包含映射表和顺序
struct BatchDataResult {
    // Date -> Data
    std::map<std::string, DailyReportData> data_map;
    // 保持原始日期的顺序 <Date, Year, Month>
    std::vector<std::tuple<std::string, int, int>> date_order;
};

class BatchDayDataFetcher {
public:
    explicit BatchDayDataFetcher(sqlite3* db);
    
    // 执行批量查询，返回组装好的数据对象
    BatchDataResult fetch_all_data();

private:
    sqlite3* db_;

    // 内部辅助方法
    void fetch_days_metadata(BatchDataResult& result);
    void fetch_time_records(BatchDataResult& result);
};

#endif // BATCH_DAY_DATA_FETCHER_HPP