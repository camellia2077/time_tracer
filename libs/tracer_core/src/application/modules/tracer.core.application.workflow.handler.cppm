module;

#include "application/pipeline/pipeline_workflow.hpp"
#include "application/workflow_handler.hpp"

export module tracer.core.application.workflow.handler;

export import tracer.core.application.workflow.interface;

export namespace tracer::core::application::workflow {

using ::tracer::core::application::pipeline::PipelineWorkflow;
using ::tracer::core::application::workflow::WorkflowHandler;

}  // namespace tracer::core::application::workflow
