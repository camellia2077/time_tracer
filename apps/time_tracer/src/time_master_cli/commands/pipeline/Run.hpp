// time_master_cli/commands/pipeline/Run.hpp
#ifndef RUN_HPP
#define RUN_HPP

#include "time_master_cli/commands/interface/ICommand.hpp"
#include "action_handler/FileHandler.hpp" // 该命令依赖 FileHandler

class Run : public ICommand {
public:
    // 通过构造函数注入它所需要的依赖
    explicit Run(FileHandler& file_handler);

    void execute(const CliParser& parser) override;
    std::string get_help() const override;

private:
    FileHandler& file_handler_;
};

#endif // RUN_HPP