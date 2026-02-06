// cli/impl/commands/query/query_command.hpp
#ifndef CLI_IMPL_COMMANDS_QUERY_QUERY_COMMAND_H_
#define CLI_IMPL_COMMANDS_QUERY_QUERY_COMMAND_H_

#include <string>

#include "application/interfaces/i_report_handler.hpp"
#include "cli/framework/interfaces/i_command.hpp"

class QueryCommand : public ICommand {
 public:
  QueryCommand(IReportHandler& report_handler, std::string default_format);

  // [新增] 实现参数定义接口
  [[nodiscard]] auto GetDefinitions() const -> std::vector<ArgDef> override;
  [[nodiscard]] auto GetHelp() const -> std::string override;

  auto Execute(const CommandParser& parser) -> void override;

 private:
  IReportHandler& report_handler_;
  std::string default_format_;
};

#endif  // CLI_IMPL_COMMANDS_QUERY_QUERY_COMMAND_H_
