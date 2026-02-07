// api/cli/impl/commands/pipeline/convert_command.hpp
#ifndef CLI_IMPL_COMMANDS_PIPELINE_CONVERT_COMMAND_H_
#define CLI_IMPL_COMMANDS_PIPELINE_CONVERT_COMMAND_H_

#include "api/cli/framework/interfaces/i_command.hpp"
#include "application/interfaces/i_workflow_handler.hpp"
#include "domain/types/date_check_mode.hpp"

class ConvertCommand : public ICommand {
 public:
  ConvertCommand(IWorkflowHandler& workflow_handler,
                 DateCheckMode default_date_check_mode,
                 bool default_save_processed_output,
                 bool default_validate_logic, bool default_validate_structure);

  [[nodiscard]] auto GetDefinitions() const -> std::vector<ArgDef> override;
  [[nodiscard]] auto GetHelp() const -> std::string override;
  [[nodiscard]] auto GetCategory() const -> std::string override {
    return "Pipeline";
  }

  auto Execute(const CommandParser& parser) -> void override;

 private:
  IWorkflowHandler& workflow_handler_;
  DateCheckMode default_date_check_mode_;
  bool default_save_processed_output_;
  bool default_validate_logic_;
  bool default_validate_structure_;
};

#endif
