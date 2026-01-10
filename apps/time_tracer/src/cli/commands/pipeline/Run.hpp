// cli/commands/pipeline/Run.hpp
#ifndef RUN_HPP
#define RUN_HPP

#include "cli/commands/interface/ICommand.hpp"
#include "core/WorkflowHandler.hpp" // [修改] 引用新头文件

class Run : public ICommand {
public:
    // [修改] 参数类型变更为 WorkflowHandler
    explicit Run(WorkflowHandler& workflow_handler);

    void execute(const CliParser& parser) override;

private:
    // [修改] 成员变量类型变更为 WorkflowHandler
    WorkflowHandler& workflow_handler_;
};

#endif // RUN_HPP