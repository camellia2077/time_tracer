module;

#include "application/pipeline/i_pipeline_workflow.hpp"
#include "application/interfaces/i_workflow_handler.hpp"

export module tracer.core.application.workflow.interface;

export namespace tracer::core::application::workflow {

using ::tracer::core::application::pipeline::IPipelineWorkflow;
using ::tracer::core::application::workflow::IWorkflowHandler;

}  // namespace tracer::core::application::workflow
