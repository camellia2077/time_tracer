// application/workflow_handler_error_mapping.cpp
#if TT_ENABLE_CPP20_MODULES
import tracer.core.application.importer.service;
import tracer.core.domain.ports.diagnostics;
#endif

#include <stdexcept>
#include <string>
#include <string_view>

#if !TT_ENABLE_CPP20_MODULES
#include "application/importer/model/import_models.hpp"
#include "domain/ports/diagnostics.hpp"
#endif

#if TT_ENABLE_CPP20_MODULES
using tracer::core::application::modimporter::ImportStats;
namespace modports = tracer::core::domain::modports;
#else
namespace modports = tracer_core::domain::ports;
#endif

namespace workflow_handler_internal {

[[nodiscard]] auto BuildPipelineFailureMessage(std::string_view base_message)
    -> std::string {
  std::string message(base_message);
  const std::string kReportDestination =
      modports::GetErrorReportDestinationLabel();
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
