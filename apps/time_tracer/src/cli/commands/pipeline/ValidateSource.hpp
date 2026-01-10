// cli/commands/pipeline/ValidateSource.hpp
#ifndef VALIDATE_SOURCE_HPP
#define VALIDATE_SOURCE_HPP

#include "cli/commands/interface/ICommand.hpp"
#include "action_handler/WorkflowHandler.hpp" // [修改] 引用新头文件

class ValidateSource : public ICommand {
public:
    // [修改] 参数类型变更
    explicit ValidateSource(WorkflowHandler& workflow_handler);
    void execute(const CliParser& parser) override;
private:
    // [修改] 成员变量类型变更
    WorkflowHandler& workflow_handler_;
};

#endif // VALIDATE_SOURCE_HPP