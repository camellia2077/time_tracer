// cli/impl/app/app_context.hpp
#ifndef CLI_IMPL_APP_APP_CONTEXT_HPP_
#define CLI_IMPL_APP_APP_CONTEXT_HPP_

// [修改] 前向声明类名变更
class WorkflowHandler;
class ReportHandler;

struct AppContext {
    // [修改] 成员类型和名称变更，以匹配重构后的 WorkflowHandler
    WorkflowHandler* workflow_handler = nullptr;
    ReportHandler* report_handler = nullptr;
};

#endif // CLI_IMPL_APP_APP_CONTEXT_HPP_