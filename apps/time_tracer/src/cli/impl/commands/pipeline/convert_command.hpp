// cli/impl/commands/pipeline/convert_command.hpp
#ifndef CLI_IMPL_COMMANDS_PIPELINE_CONVERT_COMMAND_HPP_
#define CLI_IMPL_COMMANDS_PIPELINE_CONVERT_COMMAND_HPP_

#include "cli/framework/interfaces/i_command.hpp"
#include "core/WorkflowHandler.hpp"

class ConvertCommand : public ICommand {
public:
    explicit ConvertCommand(WorkflowHandler& workflow_handler);
    // [Fix] const CommandParser&
    void execute(const CommandParser& parser) override; 
private:
    WorkflowHandler& workflow_handler_;
};

#endif // CLI_IMPL_COMMANDS_PIPELINE_CONVERT_COMMAND_HPP_