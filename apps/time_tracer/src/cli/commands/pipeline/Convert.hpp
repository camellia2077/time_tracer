// cli/commands/pipeline/Convert.hpp
#ifndef CONVERT_HPP
#define CONVERT_HPP

#include "cli/commands/interface/ICommand.hpp"
#include "core/WorkflowHandler.hpp" // [修改] 引用新头文件

class Convert : public ICommand {
public:
    // [修改] 参数类型变更为 WorkflowHandler
    explicit Convert(WorkflowHandler& workflow_handler);
    void execute(const CliParser& parser) override;
private:
    // [修改] 成员变量类型变更为 WorkflowHandler
    WorkflowHandler& workflow_handler_;
};

#endif // CONVERT_HPP