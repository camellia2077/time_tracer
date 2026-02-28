// application/workflow_handler_error_mapping.cpp
#include <stdexcept>
#include <string>
#include <string_view>

#include "application/importer/model/import_models.hpp"
#include "domain/ports/diagnostics.hpp"

namespace workflow_handler_internal {

[[nodiscard]] auto BuildPipelineFailureMessage(std::string_view base_message)
    -> std::string {
  std::string message(base_message);
  const std::string kReportDestination =
      tracer_core::domain::ports::GetErrorReportDestinationLabel();
  if (!kReportDestination.empty() && kReportDestination != "disabled") {
    message += "\nFull error report: " + kReportDestination;
  }
  return message;
}

auto ThrowIfImportTaskFailed(const ImportStats& stats,
                             std::string_view default_message) -> void {
  if (stats.db_open_success && stats.transaction_success) {
    return;
  }

  throw std::runtime_error(stats.error_message.empty()
                               ? std::string(default_message)
                               : stats.error_message);
}

}  // namespace workflow_handler_internal
