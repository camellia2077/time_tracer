// infrastructure/io/processed_data_io_noop.cpp
#include <string>

#include "application/ports/logger.hpp"
#include "infrastructure/io/processed_data_io.hpp"

namespace infrastructure::io {

auto ProcessedDataLoader::LoadDailyLogs(const std::string& processed_path)
    -> time_tracer::application::ports::ProcessedDataLoadResult {
  time_tracer::application::ports::LogWarn(
      "[ProcessedDataLoader] Processed JSON I/O is disabled by "
      "TT_ENABLE_PROCESSED_JSON_IO=OFF. Skip load: " +
      processed_path);
  return {};
}

auto ProcessedDataWriter::Write(
    const std::map<std::string, std::vector<DailyLog>>& data,
    const std::filesystem::path& output_root)
    -> std::vector<std::filesystem::path> {
  time_tracer::application::ports::LogWarn(
      "[ProcessedDataWriter] Processed JSON I/O is disabled by "
      "TT_ENABLE_PROCESSED_JSON_IO=OFF. Skip write to: " +
      output_root.string() + ", month buckets: " + std::to_string(data.size()));
  return {};
}

auto ProcessedDataStorage::WriteProcessedData(
    const std::map<std::string, std::vector<DailyLog>>& data,
    const std::filesystem::path& output_root)
    -> std::vector<std::filesystem::path> {
  return ProcessedDataWriter::Write(data, output_root);
}

}  // namespace infrastructure::io
