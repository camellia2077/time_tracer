// application/ports/i_processed_data_storage.hpp
#ifndef APPLICATION_PORTS_I_PROCESSED_DATA_STORAGE_H_
#define APPLICATION_PORTS_I_PROCESSED_DATA_STORAGE_H_

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "domain/model/daily_log.hpp"

namespace tracer_core::application::ports {

class IProcessedDataStorage {
 public:
  virtual ~IProcessedDataStorage() = default;

  virtual auto WriteProcessedData(
      const std::map<std::string, std::vector<DailyLog>>& data,
      const std::filesystem::path& output_root)
      -> std::vector<std::filesystem::path> = 0;
};

}  // namespace tracer_core::application::ports

#endif  // APPLICATION_PORTS_I_PROCESSED_DATA_STORAGE_H_
