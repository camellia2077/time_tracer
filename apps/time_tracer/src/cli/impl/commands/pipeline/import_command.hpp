// cli/impl/commands/pipeline/import_command.hpp
#ifndef CLI_IMPL_COMMANDS_PIPELINE_IMPORT_COMMAND_H_
#define CLI_IMPL_COMMANDS_PIPELINE_IMPORT_COMMAND_H_

#include "application/interfaces/i_workflow_handler.hpp"
#include "cli/framework/interfaces/i_command.hpp"

class ImportCommand : public ICommand {
 public:
  explicit ImportCommand(IWorkflowHandler& workflow_handler);

  std::vector<ArgDef> get_definitions() const override;
  std::string get_help() const override;
  void execute(const CommandParser& parser) override;

 private:
  IWorkflowHandler& workflow_handler_;
};

#endif
