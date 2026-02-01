// cli/impl/commands/pipeline/ingest_command.hpp
#ifndef CLI_IMPL_COMMANDS_PIPELINE_INGEST_COMMAND_H_
#define CLI_IMPL_COMMANDS_PIPELINE_INGEST_COMMAND_H_

#include "application/interfaces/i_workflow_handler.hpp"
#include "cli/framework/interfaces/i_command.hpp"

class IngestCommand : public ICommand {
 public:
  explicit IngestCommand(IWorkflowHandler& workflow_handler);

  // 必须在此处声明，否则 cpp 中的实现会报错
  std::vector<ArgDef> get_definitions() const override;
  std::string get_help() const override;

  void execute(const CommandParser& parser) override;

 private:
  IWorkflowHandler& workflow_handler_;
};

#endif
