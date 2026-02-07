// importer/import_service.hpp
#ifndef IMPORTER_IMPORT_SERVICE_H_
#define IMPORTER_IMPORT_SERVICE_H_

#include <map>
#include <string>
#include <vector>

#include "application/importer/model/import_stats.hpp"
#include "domain/model/daily_log.hpp"

class ImportService {
 public:
  explicit ImportService(std::string db_path);
  // [保留] 仅保留处理内存对象（结构体）的接口
  auto ImportFromMemory(
      const std::map<std::string, std::vector<DailyLog>>& data_map)
      -> ImportStats;

 private:
  std::string db_path_;
};

#endif  // IMPORTER_IMPORT_SERVICE_H_
