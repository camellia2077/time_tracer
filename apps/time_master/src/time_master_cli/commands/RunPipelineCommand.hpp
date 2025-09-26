// time_master_cli/commands/RunPipelineCommand.hpp
#ifndef RUN_PIPELINE_COMMAND_HPP
#define RUN_PIPELINE_COMMAND_HPP

#include "ICommand.hpp"
#include "action_handler/FileHandler.hpp" // 该命令依赖 FileHandler

class RunPipelineCommand : public ICommand {
public:
    // 通过构造函数注入它所需要的依赖
    explicit RunPipelineCommand(FileHandler& file_handler);

    void execute(const CliParser& parser) override;
    std::string get_help() const override;

private:
    FileHandler& file_handler_;
};

#endif // RUN_PIPELINE_COMMAND_HPP