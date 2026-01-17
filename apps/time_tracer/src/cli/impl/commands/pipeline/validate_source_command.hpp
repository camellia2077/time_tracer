// cli/impl/commands/pipeline/validate_source_command.hpp
#ifndef CLI_IMPL_COMMANDS_PIPELINE_VALIDATE_SOURCE_COMMAND_HPP_
#define CLI_IMPL_COMMANDS_PIPELINE_VALIDATE_SOURCE_COMMAND_HPP_

#include "cli/framework/interfaces/i_command.hpp"
#include "core/WorkflowHandler.hpp"

class ValidateSourceCommand : public ICommand {
public:
    explicit ValidateSourceCommand(WorkflowHandler& workflow_handler);
    void execute(const CommandParser& parser) override;
private:
    WorkflowHandler& workflow_handler_;
};

#endif // CLI_IMPL_COMMANDS_PIPELINE_VALIDATE_SOURCE_COMMAND_HPP_