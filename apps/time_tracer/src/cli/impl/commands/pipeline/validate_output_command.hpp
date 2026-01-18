// cli/impl/commands/pipeline/validate_output_command.hpp
#ifndef CLI_IMPL_COMMANDS_PIPELINE_VALIDATE_OUTPUT_COMMAND_HPP_
#define CLI_IMPL_COMMANDS_PIPELINE_VALIDATE_OUTPUT_COMMAND_HPP_

#include "cli/framework/interfaces/i_command.hpp"
#include "core/workflow_handler.hpp"

class ValidateOutputCommand : public ICommand {
public:
    explicit ValidateOutputCommand(WorkflowHandler& workflow_handler);
    void execute(const CommandParser& parser) override;
private:
    WorkflowHandler& workflow_handler_;
};

#endif // CLI_IMPL_COMMANDS_PIPELINE_VALIDATE_OUTPUT_COMMAND_HPP_