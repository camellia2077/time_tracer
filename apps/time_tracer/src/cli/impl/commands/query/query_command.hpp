// cli/impl/commands/query/query_command.hpp
#ifndef CLI_IMPL_COMMANDS_QUERY_QUERY_COMMAND_H_
#define CLI_IMPL_COMMANDS_QUERY_QUERY_COMMAND_H_

#include "cli/framework/interfaces/i_command.hpp"
#include "application/interfaces/i_report_handler.hpp"

class QueryCommand : public ICommand {
 public:
  explicit QueryCommand(IReportHandler& report_handler);

  // [新增] 实现参数定义接口
  std::vector<ArgDef> get_definitions() const override;
  std::string get_help() const override;
  void execute(const CommandParser& parser) override;

 private:
  IReportHandler& report_handler_;
};

#endif  // CLI_IMPL_COMMANDS_QUERY_QUERY_COMMAND_H_
