// application/interfaces/i_workflow_handler.hpp
#ifndef APPLICATION_INTERFACES_I_WORKFLOW_HANDLER_H_
#define APPLICATION_INTERFACES_I_WORKFLOW_HANDLER_H_

#include <map>
#include <string>
#include <vector>

#include "domain/types/date_check_mode.hpp"
#include "domain/types/ingest_mode.hpp"

struct AppOptions;
struct DailyLog;

namespace tracer::core::application::workflow {

class IWorkflowHandler {
 public:
  virtual ~IWorkflowHandler() = default;

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
  virtual auto RunIngestReplacingAll(const std::string& source_path,
                                     DateCheckMode date_check_mode,
                                     bool save_processed = false) -> void = 0;

  virtual auto RunValidateStructure(const std::string& source_path) -> void = 0;
  virtual auto RunValidateLogic(const std::string& source_path,
                                DateCheckMode date_check_mode) -> void = 0;
};

}  // namespace tracer::core::application::workflow

using IWorkflowHandler = tracer::core::application::workflow::IWorkflowHandler;

#endif  // APPLICATION_INTERFACES_I_WORKFLOW_HANDLER_H_
