module;

#include "application/interfaces/i_workflow_handler.hpp"

export module tracer.core.application.workflow.interface;

export namespace tracer::core::application::workflow {

using ::tracer::core::application::workflow::IWorkflowHandler;

}  // namespace tracer::core::application::workflow

export namespace tracer::core::application::modworkflow {

using ::IWorkflowHandler;

}  // namespace tracer::core::application::modworkflow
