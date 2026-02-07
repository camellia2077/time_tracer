// api/cli/impl/commands/export/export_command.hpp
#ifndef CLI_IMPL_COMMANDS_EXPORT_EXPORT_COMMAND_H_
#define CLI_IMPL_COMMANDS_EXPORT_EXPORT_COMMAND_H_

#include <string>

#include "api/cli/framework/interfaces/i_command.hpp"
#include "application/interfaces/i_report_handler.hpp"

class ExportCommand : public ICommand {
 public:
  ExportCommand(IReportHandler& report_handler, std::string default_format);

  [[nodiscard]] auto GetDefinitions() const -> std::vector<ArgDef> override;
  [[nodiscard]] auto GetHelp() const -> std::string override;
  [[nodiscard]] auto GetCategory() const -> std::string override {
    return "Export";
  }

  auto Execute(const CommandParser& parser) -> void override;

 private:
  IReportHandler& report_handler_;
  std::string default_format_;
};

#endif
