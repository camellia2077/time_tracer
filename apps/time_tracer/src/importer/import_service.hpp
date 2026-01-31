// importer/import_service.hpp
#ifndef IMPORTER_IMPORT_SERVICE_H_
#define IMPORTER_IMPORT_SERVICE_H_

#include <map>
#include <string>
#include <vector>

#include "domain/model/daily_log.hpp"
#include "importer/model/import_stats.hpp"

class ImportService {
 public:
  explicit ImportService(std::string db_path);
  // [保留] 仅保留处理内存对象（结构体）的接口
  ImportStats import_from_memory(
      const std::map<std::string, std::vector<DailyLog>>& data_map);

 private:
  std::string db_path_;
};

#endif  // IMPORTER_IMPORT_SERVICE_H_
