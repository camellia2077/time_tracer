// cli/commands/pipeline/Import.hpp
#ifndef IMPORT_HPP
#define IMPORT_HPP

#include "cli/commands/interface/ICommand.hpp"
#include "core/WorkflowHandler.hpp" // [修改] 引用新头文件

class Import : public ICommand {
public:
    // [修改] 参数类型变更
    explicit Import(WorkflowHandler& workflow_handler);
    void execute(const CliParser& parser) override;
private:
    // [修改] 成员变量类型变更
    WorkflowHandler& workflow_handler_;
};

#endif // IMPORT_HPP