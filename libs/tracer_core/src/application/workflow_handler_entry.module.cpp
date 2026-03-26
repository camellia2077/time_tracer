#include <filesystem>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "application/workflow_handler.hpp"
#include "application/pipeline/pipeline_orchestrator.hpp"
#include "domain/ports/diagnostics.hpp"
#include "domain/types/app_options.hpp"
#include "domain/types/date_check_mode.hpp"

namespace fs = std::filesystem;

struct AppOptions;
struct DailyLog;

namespace tracer_core::application::ports {
class IConverterConfigProvider;
class IDatabaseHealthChecker;
class IIngestInputProvider;
class IProcessedDataLoader;
class IProcessedDataStorage;
class ITimeSheetRepository;
class IValidationIssueReporter;
}  // namespace tracer_core::application::ports

namespace app_ports = tracer_core::application::ports;
namespace app_pipeline = tracer::core::application::pipeline;
namespace modports = tracer::core::domain::ports;

using app_pipeline::PipelineOrchestrator;
using tracer::core::domain::types::AppOptions;
using tracer::core::domain::types::DateCheckMode;

namespace workflow_handler_internal {
[[nodiscard]] auto BuildPipelineFailureMessage(std::string_view base_message)
    -> std::string;
}  // namespace workflow_handler_internal

#include "application/internal/workflow_handler_entry_impl.inc"
