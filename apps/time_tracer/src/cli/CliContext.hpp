// cli/CliContext.hpp
#ifndef CLI_CONTEXT_HPP
#define CLI_CONTEXT_HPP

// [修改] 前向声明类名变更
class WorkflowHandler;
class ReportHandler;

struct CliContext {
    // [修改] 成员类型和名称变更，以匹配重构后的 WorkflowHandler
    WorkflowHandler* workflow_handler = nullptr;
    ReportHandler* report_handler = nullptr;
};

#endif // CLI_CONTEXT_HPP