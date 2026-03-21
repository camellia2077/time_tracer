#ifndef APPLICATION_INTERFACES_I_WORKFLOW_HANDLER_H_
#define APPLICATION_INTERFACES_I_WORKFLOW_HANDLER_H_

#include "application/pipeline/i_pipeline_workflow.hpp"

namespace tracer::core::application::workflow {

class IWorkflowHandler : public ::tracer::core::application::pipeline::IPipelineWorkflow {
 public:
  ~IWorkflowHandler() override = default;
};

}  // namespace tracer::core::application::workflow

using IWorkflowHandler = tracer::core::application::workflow::IWorkflowHandler;

#endif  // APPLICATION_INTERFACES_I_WORKFLOW_HANDLER_H_
