// application/workflow/workflow_handler.hpp
#ifndef APPLICATION_WORKFLOW_WORKFLOW_HANDLER_H_
#define APPLICATION_WORKFLOW_WORKFLOW_HANDLER_H_

#include "application/ports/file_system.hpp"
#include "application/ports/log_generator_factory.hpp"
#include "application/insights/insights_handler.hpp"
#include "common/app_context.hpp"

namespace App {

class WorkflowHandler {
 public:
  WorkflowHandler(FileSystem& file_system,
                  ILogGeneratorFactory& generator_factory);
  int run(const Core::AppContext& context, InsightsHandler& insights_handler);

 private:
  FileSystem& file_system_;
  ILogGeneratorFactory& generator_factory_;
};

}  // namespace App

#endif  // APPLICATION_WORKFLOW_WORKFLOW_HANDLER_H_
