// application/pipeline/importer/import_service.hpp
#ifndef APPLICATION_IMPORTER_IMPORT_SERVICE_H_
#define APPLICATION_IMPORTER_IMPORT_SERVICE_H_

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "application/pipeline/importer/model/import_models.hpp"

struct DailyLog;
namespace tracer_core::application::ports {
class ITimeSheetRepository;
}  // namespace tracer_core::application::ports

struct ReplaceAllTarget {};

class ImportService {
 public:
  explicit ImportService(
      tracer_core::application::ports::ITimeSheetRepository& repository);
  // [保留] 仅保留处理内存对象（结构体）的接口
  auto ImportFromMemory(
      const std::map<std::string, std::vector<DailyLog>>& data_map,
      const std::optional<ReplaceMonthTarget>& replace_month_target =
          std::nullopt,
      const std::optional<ReplaceAllTarget>& replace_all_target = std::nullopt)
      -> ImportStats;

 private:
  tracer_core::application::ports::ITimeSheetRepository& repository_;
};

#endif  // APPLICATION_IMPORTER_IMPORT_SERVICE_H_
