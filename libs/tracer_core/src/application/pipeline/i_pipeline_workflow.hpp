#ifndef APPLICATION_PIPELINE_I_PIPELINE_WORKFLOW_HPP_
#define APPLICATION_PIPELINE_I_PIPELINE_WORKFLOW_HPP_

#include <map>
#include <string>
#include <vector>

#include "application/dto/pipeline_responses.hpp"
#include "domain/types/date_check_mode.hpp"
#include "domain/types/ingest_mode.hpp"

struct AppOptions;
struct DailyLog;

namespace tracer::core::application::pipeline {

class IPipelineWorkflow {
 public:
  virtual ~IPipelineWorkflow() = default;

  virtual auto RunConverter(const std::string& input_path,
                            const AppOptions& options) -> void = 0;
  virtual auto RunDatabaseImport(const std::string& processed_path_str)
      -> void = 0;
  virtual auto RunDatabaseImportFromMemory(
      const std::map<std::string, std::vector<DailyLog>>& data_map) -> void = 0;
  virtual auto RunIngest(const std::string& source_path,
                         DateCheckMode date_check_mode,
                         bool save_processed = false,
                         IngestMode ingest_mode = IngestMode::kStandard)
      -> void = 0;
  virtual auto RunIngestSyncStatusQuery(
      const tracer_core::core::dto::IngestSyncStatusRequest& request)
      -> tracer_core::core::dto::IngestSyncStatusOutput = 0;
  virtual auto ClearIngestSyncStatus() -> void = 0;
  virtual auto RunIngestReplacingAll(const std::string& source_path,
                                     DateCheckMode date_check_mode,
                                     bool save_processed = false) -> void = 0;
  virtual auto RunValidateStructure(const std::string& source_path) -> void = 0;
  virtual auto RunValidateLogic(const std::string& source_path,
                                DateCheckMode date_check_mode) -> void = 0;
  virtual auto InstallActiveConverterConfig(
      const std::string& source_main_config_path,
      const std::string& target_main_config_path) -> void = 0;
};

}  // namespace tracer::core::application::pipeline

using IPipelineWorkflow =
    tracer::core::application::pipeline::IPipelineWorkflow;

#endif  // APPLICATION_PIPELINE_I_PIPELINE_WORKFLOW_HPP_
