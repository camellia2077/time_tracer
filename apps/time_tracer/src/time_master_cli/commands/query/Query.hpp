// time_master_cli/commands/query/Query.hpp
#ifndef QUERY_HPP
#define QUERY_HPP

#include "time_master_cli/commands/interface/ICommand.hpp"
#include "action_handler/ReportHandler.hpp"

class Query : public ICommand {
public:
    explicit Query(ReportHandler& report_handler);
    void execute(const CliParser& parser) override;

private:
    ReportHandler& report_handler_;
};

#endif // QUERY_HPP