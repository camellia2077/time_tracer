// cli/impl/commands/pipeline/import_command.hpp
#ifndef CLI_IMPL_COMMANDS_PIPELINE_IMPORT_COMMAND_HPP_
#define CLI_IMPL_COMMANDS_PIPELINE_IMPORT_COMMAND_HPP_

#include "cli/framework/interfaces/i_command.hpp"
#include "core/workflow_handler.hpp"

class ImportCommand : public ICommand { // [Fix] ICommand
public:
    explicit ImportCommand(WorkflowHandler& workflow_handler);
    void execute(const CommandParser& parser) override; // [Fix] CommandParser
private:
    WorkflowHandler& workflow_handler_;
};

#endif // CLI_IMPL_COMMANDS_PIPELINE_IMPORT_COMMAND_HPP_