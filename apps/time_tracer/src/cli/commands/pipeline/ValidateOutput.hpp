// cli/commands/pipeline/ValidateOutput.hpp
#ifndef VALIDATE_OUTPUT_HPP
#define VALIDATE_OUTPUT_HPP

#include "cli/commands/interface/ICommand.hpp"
#include "action_handler/WorkflowHandler.hpp" // [修改] 引用新头文件

class ValidateOutput : public ICommand {
public:
    // [修改] 参数类型变更
    explicit ValidateOutput(WorkflowHandler& workflow_handler);
    void execute(const CliParser& parser) override;
private:
    // [修改] 成员变量类型变更
    WorkflowHandler& workflow_handler_;
};

#endif // VALIDATE_OUTPUT_HPP