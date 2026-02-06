// cli/impl/commands/pipeline/ingest_command.hpp
#ifndef CLI_IMPL_COMMANDS_PIPELINE_INGEST_COMMAND_H_
#define CLI_IMPL_COMMANDS_PIPELINE_INGEST_COMMAND_H_

#include "application/interfaces/i_workflow_handler.hpp"
#include "cli/framework/interfaces/i_command.hpp"
#include "domain/types/date_check_mode.hpp"

class IngestCommand : public ICommand {
 public:
  IngestCommand(IWorkflowHandler& workflow_handler,
                DateCheckMode default_date_check_mode,
                bool default_save_processed_output);

  // 必须在此处声明，否则 cpp 中的实现会报错
  [[nodiscard]] auto GetDefinitions() const -> std::vector<ArgDef> override;
  [[nodiscard]] auto GetHelp() const -> std::string override;

  void Execute(const CommandParser& parser) override;

 private:
  IWorkflowHandler& workflow_handler_;
  DateCheckMode default_date_check_mode_;
  bool default_save_processed_output_;
};

#endif
