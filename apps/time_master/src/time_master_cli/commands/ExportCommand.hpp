// time_master_cli/commands/ExportCommand.hpp
#ifndef EXPORT_COMMAND_HPP
#define EXPORT_COMMAND_HPP

#include "ICommand.hpp"
#include "action_handler/ReportHandler.hpp"

class ExportCommand : public ICommand {
public:
    explicit ExportCommand(ReportHandler& report_handler);
    void execute(const CliParser& parser) override;

private:
    ReportHandler& report_handler_;
};

#endif // EXPORT_COMMAND_HPP