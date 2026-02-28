// api/cli/impl/app/app_context.hpp
#ifndef API_CLI_IMPL_APP_APP_CONTEXT_H_
#define API_CLI_IMPL_APP_APP_CONTEXT_H_

#include <memory>

#include "application/dto/cli_config.hpp"

// 前向声明接口
class ITracerCoreApi;

/**
 * @brief 服务容器，持有应用程序核心服务的共享实例。
 */
struct AppContext {
  std::shared_ptr<ITracerCoreApi> core_api;

  // CLI command default settings mapped from configuration.
  tracer_core::application::dto::CliConfig config;
};

#endif // API_CLI_IMPL_APP_APP_CONTEXT_H_
