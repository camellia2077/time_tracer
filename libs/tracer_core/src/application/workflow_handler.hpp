// application/workflow_handler.hpp
#ifndef APPLICATION_WORKFLOW_HANDLER_H_
#define APPLICATION_WORKFLOW_HANDLER_H_

#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "application/interfaces/i_workflow_handler.hpp"

namespace fs = std::filesystem;

namespace tracer_core::application::ports {
class IConverterConfigProvider;
class IDatabaseHealthChecker;
class IIngestInputProvider;
class IProcessedDataLoader;
class IProcessedDataStorage;
class ITimeSheetRepository;
class IValidationIssueReporter;
}  // namespace tracer_core::application::ports

namespace tracer::core::application::workflow {

#include "application/workflow/detail/workflow_handler_decl.inc"

}  // namespace tracer::core::application::workflow

using WorkflowHandler = tracer::core::application::workflow::WorkflowHandler;

#endif  // APPLICATION_WORKFLOW_HANDLER_H_
