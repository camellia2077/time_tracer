// reports/data/model/query_data_structs.hpp
#ifndef REPORTS_DATA_MODEL_QUERY_DATA_STRUCTS_H_
#define REPORTS_DATA_MODEL_QUERY_DATA_STRUCTS_H_

#include <map>
#include <string>
#include <vector>

//  用于导出所有报告的数据结构
using FormattedGroupedReports =
    std::map<int,
             std::map<int, std::vector<std::pair<std::string, std::string>>>>;
//  用于导出所有月报的数据结构
using FormattedMonthlyReports = std::map<int, std::map<int, std::string>>;
// 用于导出所有周期报告的数据结构
using FormattedPeriodReports = std::map<int, std::string>;

#endif  // REPORTS_DATA_MODEL_QUERY_DATA_STRUCTS_H_