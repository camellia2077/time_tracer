// cli/impl/app/app_context.hpp
#ifndef CLI_IMPL_APP_APP_CONTEXT_H_
#define CLI_IMPL_APP_APP_CONTEXT_H_

#include <memory>

#include "common/config/app_config.hpp"  // [新增] 必须包含此头文件

// 前向声明接口
class IWorkflowHandler;
class IReportHandler;

/**
 * @brief 服务容器，持有应用程序核心服务的共享实例。
 */
struct AppContext {
  // 使用 shared_ptr 共享所有权，确保服务在命令执行期间存活
  std::shared_ptr<IWorkflowHandler> workflow_handler;
  std::shared_ptr<IReportHandler> report_handler;

  // [修复] 添加 config 成员，供 Command 使用
  AppConfig config;
};

#endif  // CLI_IMPL_APP_APP_CONTEXT_H_