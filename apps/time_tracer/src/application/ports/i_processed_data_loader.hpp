// application/ports/i_processed_data_loader.hpp
#ifndef APPLICATION_PORTS_I_PROCESSED_DATA_LOADER_H_
#define APPLICATION_PORTS_I_PROCESSED_DATA_LOADER_H_

#include <map>
#include <string>
#include <vector>

#include "domain/model/daily_log.hpp"

namespace time_tracer::application::ports {

struct ProcessedDataLoadError {
  std::string source;
  std::string message;
};

struct ProcessedDataLoadResult {
  std::map<std::string, std::vector<DailyLog>> data_by_source;
  std::vector<ProcessedDataLoadError> errors;
};

class IProcessedDataLoader {
 public:
  virtual ~IProcessedDataLoader() = default;

  virtual auto LoadDailyLogs(const std::string& processed_path)
      -> ProcessedDataLoadResult = 0;
};

}  // namespace time_tracer::application::ports

#endif  // APPLICATION_PORTS_I_PROCESSED_DATA_LOADER_H_
