#ifndef CLI_IMPL_COMMANDS_PIPELINE_INGEST_COMMAND_HPP_
#define CLI_IMPL_COMMANDS_PIPELINE_INGEST_COMMAND_HPP_

#include "cli/framework/interfaces/i_command.hpp"
#include "core/interfaces/i_workflow_handler.hpp"

class IngestCommand : public ICommand {
public:
    explicit IngestCommand(IWorkflowHandler& workflow_handler);
    
    // 必须在此处声明，否则 cpp 中的实现会报错
    std::vector<ArgDef> get_definitions() const override;
    std::string get_help() const override;

    void execute(const CommandParser& parser) override;

private:
    IWorkflowHandler& workflow_handler_;
};

#endif