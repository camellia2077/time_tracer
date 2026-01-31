// importer/model/import_stats.hpp
#ifndef IMPORTER_MODEL_IMPORT_STATS_H_
#define IMPORTER_MODEL_IMPORT_STATS_H_

#include <string>
#include <vector>

// 用于在 Service 层和 Facade 层之间传递执行结果
struct ImportStats {
  size_t total_files = 0;
  size_t successful_files = 0;
  std::vector<std::string> failed_files;

  // 耗时统计 (秒)
  double parsing_duration_s = 0.0;
  double db_insertion_duration_s = 0.0;

  // 数据库事务状态
  bool db_open_success = false;
  bool transaction_success = false;
  std::string error_message;  // 全局错误信息
};

#endif  // IMPORTER_MODEL_IMPORT_STATS_H_