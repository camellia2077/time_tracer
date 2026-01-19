#ifndef CLI_IMPL_COMMANDS_EXPORT_EXPORT_COMMAND_HPP_
#define CLI_IMPL_COMMANDS_EXPORT_EXPORT_COMMAND_HPP_

#include "cli/framework/interfaces/i_command.hpp"
#include "core/interfaces/i_report_handler.hpp" 

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