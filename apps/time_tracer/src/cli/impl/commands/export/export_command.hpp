// cli/impl/commands/export/export_command.hpp
#ifndef CLI_IMPL_COMMANDS_EXPORT_EXPORT_COMMAND_H_
#define CLI_IMPL_COMMANDS_EXPORT_EXPORT_COMMAND_H_

#include "application/interfaces/i_report_handler.hpp"
#include "cli/framework/interfaces/i_command.hpp"

class ExportCommand : public ICommand {
 public:
  explicit ExportCommand(IReportHandler& report_handler);

  std::vector<ArgDef> get_definitions() const override;
  std::string get_help() const override;
  void execute(const CommandParser& parser) override;

 private:
  IReportHandler& report_handler_;
};

#endif
