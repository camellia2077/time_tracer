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

#include "application/workflow/detail/i_workflow_handler_decl.inc"

}  // namespace tracer::core::application::workflow

using IWorkflowHandler = tracer::core::application::workflow::IWorkflowHandler;

#endif  // APPLICATION_INTERFACES_I_WORKFLOW_HANDLER_H_
