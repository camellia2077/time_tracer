// application/importer/import_service.hpp
#ifndef APPLICATION_IMPORTER_IMPORT_SERVICE_H_
#define APPLICATION_IMPORTER_IMPORT_SERVICE_H_

#include <map>
#include <vector>

#include "application/importer/model/import_models.hpp"
#include "application/ports/i_time_sheet_repository.hpp"
#include "domain/model/daily_log.hpp"

class ImportService {
 public:
  explicit ImportService(
      time_tracer::application::ports::ITimeSheetRepository& repository);
  // [保留] 仅保留处理内存对象（结构体）的接口
  auto ImportFromMemory(
      const std::map<std::string, std::vector<DailyLog>>& data_map)
      -> ImportStats;

 private:
  time_tracer::application::ports::ITimeSheetRepository& repository_;
};

#endif  // APPLICATION_IMPORTER_IMPORT_SERVICE_H_
