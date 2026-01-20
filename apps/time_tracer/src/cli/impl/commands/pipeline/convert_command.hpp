// cli/impl/commands/pipeline/convert_command.hpp
#ifndef CLI_IMPL_COMMANDS_PIPELINE_CONVERT_COMMAND_HPP_
#define CLI_IMPL_COMMANDS_PIPELINE_CONVERT_COMMAND_HPP_

#include "cli/framework/interfaces/i_command.hpp"
#include "core/interfaces/i_workflow_handler.hpp"

class ConvertCommand : public ICommand {
public:
    explicit ConvertCommand(IWorkflowHandler& workflow_handler);
    
    std::vector<ArgDef> get_definitions() const override;
    std::string get_help() const override;
    void execute(const CommandParser& parser) override; 
private:
    IWorkflowHandler& workflow_handler_;
};

#endif