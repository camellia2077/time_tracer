// cli/impl/commands/export/export_command.hpp
#ifndef CLI_IMPL_COMMANDS_EXPORT_EXPORT_COMMAND_HPP_
#define CLI_IMPL_COMMANDS_EXPORT_EXPORT_COMMAND_HPP_

#include "cli/framework/interfaces/i_command.hpp"
#include "core/report_handler.hpp"

class ExportCommand : public ICommand { 
public: 
    explicit ExportCommand(ReportHandler& report_handler);
    void execute(const CommandParser& parser) override;

private:
    ReportHandler& report_handler_;
};

#endif // CLI_IMPL_COMMANDS_EXPORT_EXPORT_COMMAND_HPP_