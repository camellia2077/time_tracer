// infra/io/processed_data_io_noop.cpp
#include <string>

#include "application/runtime_bridge/logger.hpp"
#include "infra/io/internal/runtime_adapter_types.hpp"

namespace infrastructure::io::internal {

auto ProcessedDataLoaderAdapter::LoadDailyLogs(
    const std::string& processed_path)
    -> tracer_core::application::ports::ProcessedDataLoadResult {
  tracer_core::application::runtime_bridge::LogWarn(
      "[ProcessedDataLoader] Processed JSON I/O is disabled by "
      "TT_ENABLE_PROCESSED_JSON_IO=OFF. Skip load: " +
      processed_path);
  return {};
}

auto ProcessedDataWriter::Write(
    const std::map<std::string, std::vector<DailyLog>>& data,
    const std::filesystem::path& output_root)
    -> std::vector<std::filesystem::path> {
  tracer_core::application::runtime_bridge::LogWarn(
      "[ProcessedDataWriter] Processed JSON I/O is disabled by "
      "TT_ENABLE_PROCESSED_JSON_IO=OFF. Skip write to: " +
      output_root.string() + ", month buckets: " + std::to_string(data.size()));
  return {};
}

auto ProcessedDataStorageAdapter::WriteProcessedData(
    const std::map<std::string, std::vector<DailyLog>>& data,
    const std::filesystem::path& output_root)
    -> std::vector<std::filesystem::path> {
  return ProcessedDataWriter::Write(data, output_root);
}

}  // namespace infrastructure::io::internal
