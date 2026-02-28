// api/cli/impl/app/cli_application.hpp
#ifndef API_CLI_IMPL_APP_CLI_APPLICATION_H_
#define API_CLI_IMPL_APP_CLI_APPLICATION_H_

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "api/cli/framework/core/command_parser.hpp"
#include "api/cli/impl/app/app_context.hpp"
#include "application/ports/i_cli_runtime_factory.hpp"

namespace fs = std::filesystem;

class CliApplication {
public:
  CliApplication(
      const std::vector<std::string> &args,
      std::shared_ptr<tracer_core::application::ports::ICliRuntimeFactory>
          runtime_factory);
  ~CliApplication();

  [[nodiscard]] auto Execute() -> int;

private:
  CommandParser parser_;

  std::shared_ptr<AppContext> app_context_;
  std::shared_ptr<tracer_core::application::ports::ICliRuntimeFactory>
      runtime_factory_;
  std::shared_ptr<void> runtime_state_;
  bool runtime_built_ = false;

  void BuildRuntime();
  void EnsureRuntimeBuilt();
};

#endif // API_CLI_IMPL_APP_CLI_APPLICATION_H_
