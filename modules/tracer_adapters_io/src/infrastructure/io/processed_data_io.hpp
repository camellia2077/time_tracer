// infrastructure/io/processed_data_io.hpp
#ifndef INFRASTRUCTURE_IO_PROCESSED_DATA_IO_H_
#define INFRASTRUCTURE_IO_PROCESSED_DATA_IO_H_

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "application/ports/i_processed_data_loader.hpp"
#include "application/ports/i_processed_data_storage.hpp"
#include "domain/model/daily_log.hpp"

namespace infrastructure::io {

class ProcessedDataLoader final
    : public time_tracer::application::ports::IProcessedDataLoader {
 public:
  auto LoadDailyLogs(const std::string& processed_path)
      -> time_tracer::application::ports::ProcessedDataLoadResult override;
};

class ProcessedDataWriter {
 public:
  static auto Write(const std::map<std::string, std::vector<DailyLog>>& data,
                    const std::filesystem::path& output_root)
      -> std::vector<std::filesystem::path>;
};

class ProcessedDataStorage final
    : public time_tracer::application::ports::IProcessedDataStorage {
 public:
  auto WriteProcessedData(
      const std::map<std::string, std::vector<DailyLog>>& data,
      const std::filesystem::path& output_root)
      -> std::vector<std::filesystem::path> override;
};

}  // namespace infrastructure::io

#endif  // INFRASTRUCTURE_IO_PROCESSED_DATA_IO_H_
