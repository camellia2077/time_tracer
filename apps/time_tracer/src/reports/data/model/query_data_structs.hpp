// reports/data/model/query_data_structs.hpp
#ifndef REPORTS_DATA_MODEL_QUERY_DATA_STRUCTS_HPP_
#define REPORTS_DATA_MODEL_QUERY_DATA_STRUCTS_HPP_

#include <string>
#include <vector>
#include <map>

//  用于导出所有报告的数据结构
using FormattedGroupedReports = std::map<int, std::map<int, std::vector<std::pair<std::string, std::string>>>>;
//  用于导出所有月报的数据结构
using FormattedMonthlyReports = std::map<int, std::map<int, std::string>>;
// 用于导出所有周期报告的数据结构
using FormattedPeriodReports = std::map<int, std::string>;

#endif // REPORTS_DATA_MODEL_QUERY_DATA_STRUCTS_HPP_