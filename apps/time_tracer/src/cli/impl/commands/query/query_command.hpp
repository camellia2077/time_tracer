// cli/impl/commands/query/query_command.hpp
#ifndef CLI_IMPL_COMMANDS_QUERY_QUERY_COMMAND_HPP_
#define CLI_IMPL_COMMANDS_QUERY_QUERY_COMMAND_HPP_

#include "cli/framework/interfaces/i_command.hpp"
#include "core/reporting/report_handler.hpp"

class QueryCommand : public ICommand {
public:
    explicit QueryCommand(ReportHandler& report_handler);
    void execute(const CommandParser& parser) override;

private:
    ReportHandler& report_handler_;
};

#endif // CLI_IMPL_COMMANDS_QUERY_QUERY_COMMAND_HPP_