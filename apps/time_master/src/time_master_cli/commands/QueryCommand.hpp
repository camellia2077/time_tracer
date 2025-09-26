// time_master_cli/commands/QueryCommand.hpp
#ifndef QUERY_COMMAND_HPP
#define QUERY_COMMAND_HPP

#include "ICommand.hpp"
#include "action_handler/ReportHandler.hpp"

class QueryCommand : public ICommand {
public:
    explicit QueryCommand(ReportHandler& report_handler);
    void execute(const CliParser& parser) override;

private:
    ReportHandler& report_handler_;
};

#endif // QUERY_COMMAND_HPP