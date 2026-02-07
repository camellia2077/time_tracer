// api/cli/impl/commands/query/query_command.hpp
#ifndef CLI_IMPL_COMMANDS_QUERY_QUERY_COMMAND_H_
#define CLI_IMPL_COMMANDS_QUERY_QUERY_COMMAND_H_

#include <string>

#include "api/cli/framework/interfaces/i_command.hpp"
#include "application/interfaces/i_report_handler.hpp"

class QueryCommand : public ICommand {
 public:
  QueryCommand(IReportHandler& report_handler, std::string default_format,
               std::string db_path);

  // [新增] 实现参数定义接口
  [[nodiscard]] auto GetDefinitions() const -> std::vector<ArgDef> override;
  [[nodiscard]] auto GetHelp() const -> std::string override;
  [[nodiscard]] auto GetCategory() const -> std::string override {
    return "Query";
  }

  auto Execute(const CommandParser& parser) -> void override;

 private:
  IReportHandler& report_handler_;
  std::string default_format_;
  std::string db_path_;
};

#endif  // CLI_IMPL_COMMANDS_QUERY_QUERY_COMMAND_H_
