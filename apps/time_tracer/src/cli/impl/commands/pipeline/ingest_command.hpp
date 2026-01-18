// cli/impl/commands/pipeline/ingest_command.hpp
#ifndef CLI_IMPL_COMMANDS_PIPELINE_INGEST_COMMAND_HPP_
#define CLI_IMPL_COMMANDS_PIPELINE_INGEST_COMMAND_HPP_

#include "cli/framework/interfaces/i_command.hpp"
#include "core/workflow_handler.hpp"

class IngestCommand : public ICommand {
public:
    explicit IngestCommand(WorkflowHandler& workflow_handler);
    void execute(const CommandParser& parser) override;

private:
    WorkflowHandler& workflow_handler_;
};

#endif // CLI_IMPL_COMMANDS_PIPELINE_INGEST_COMMAND_HPP_