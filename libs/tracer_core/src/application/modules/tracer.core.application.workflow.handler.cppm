module;

#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "domain/types/date_check_mode.hpp"
#include "domain/types/ingest_mode.hpp"

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

export module tracer.core.application.workflow.handler;

export import tracer.core.application.workflow.interface;

export namespace tracer::core::application::workflow {

#include "application/workflow/detail/workflow_handler_decl.inc"

}  // namespace tracer::core::application::workflow

export namespace tracer::core::application::modworkflow {

using tracer::core::application::workflow::WorkflowHandler;

}  // namespace tracer::core::application::modworkflow
