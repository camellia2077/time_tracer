// infra/io/internal/runtime_adapter_types.hpp
#ifndef INFRASTRUCTURE_IO_INTERNAL_RUNTIME_ADAPTER_TYPES_H_
#define INFRASTRUCTURE_IO_INTERNAL_RUNTIME_ADAPTER_TYPES_H_

#include <filesystem>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "application/ports/i_ingest_input_provider.hpp"
#include "application/ports/i_processed_data_loader.hpp"
#include "application/ports/i_processed_data_storage.hpp"
#include "domain/model/daily_log.hpp"

namespace infrastructure::io::internal {

class TxtIngestInputProviderAdapter final
    : public tracer_core::application::ports::IIngestInputProvider {
 public:
  [[nodiscard]] auto CollectTextInputs(const std::filesystem::path& input_root,
                                       std::string_view extension) const
      -> tracer_core::application::dto::IngestInputCollection override;
};

class ProcessedDataLoaderAdapter final
    : public tracer_core::application::ports::IProcessedDataLoader {
 public:
  auto LoadDailyLogs(const std::string& processed_path)
      -> tracer_core::application::ports::ProcessedDataLoadResult override;
};

class ProcessedDataWriter {
 public:
  static auto Write(const std::map<std::string, std::vector<DailyLog>>& data,
                    const std::filesystem::path& output_root)
      -> std::vector<std::filesystem::path>;
};

class ProcessedDataStorageAdapter final
    : public tracer_core::application::ports::IProcessedDataStorage {
 public:
  auto WriteProcessedData(
      const std::map<std::string, std::vector<DailyLog>>& data,
      const std::filesystem::path& output_root)
      -> std::vector<std::filesystem::path> override;
};

}  // namespace infrastructure::io::internal

#endif  // INFRASTRUCTURE_IO_INTERNAL_RUNTIME_ADAPTER_TYPES_H_
