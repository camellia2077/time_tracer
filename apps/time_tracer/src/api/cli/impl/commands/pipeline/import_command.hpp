// api/cli/impl/commands/pipeline/import_command.hpp
#ifndef CLI_IMPL_COMMANDS_PIPELINE_IMPORT_COMMAND_H_
#define CLI_IMPL_COMMANDS_PIPELINE_IMPORT_COMMAND_H_

#include "api/cli/framework/interfaces/i_command.hpp"
#include "application/interfaces/i_workflow_handler.hpp"

class ImportCommand : public ICommand {
 public:
  explicit ImportCommand(IWorkflowHandler& workflow_handler);

  [[nodiscard]] auto GetDefinitions() const -> std::vector<ArgDef> override;
  [[nodiscard]] auto GetHelp() const -> std::string override;
  [[nodiscard]] auto GetCategory() const -> std::string override {
    return "Pipeline";
  }

  auto Execute(const CommandParser& parser) -> void override;

 private:
  IWorkflowHandler& workflow_handler_;
};

#endif
