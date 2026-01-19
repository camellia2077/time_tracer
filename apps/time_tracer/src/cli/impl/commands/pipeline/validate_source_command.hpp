#ifndef CLI_IMPL_COMMANDS_PIPELINE_VALIDATE_SOURCE_COMMAND_HPP_
#define CLI_IMPL_COMMANDS_PIPELINE_VALIDATE_SOURCE_COMMAND_HPP_

#include "cli/framework/interfaces/i_command.hpp"
#include "core/interfaces/i_workflow_handler.hpp"

class ValidateSourceCommand : public ICommand {
public:
    explicit ValidateSourceCommand(IWorkflowHandler& workflow_handler);
    
    std::vector<ArgDef> get_definitions() const override;
    std::string get_help() const override;
    void execute(const CommandParser& parser) override;
private:
    IWorkflowHandler& workflow_handler_;
};

#endif