// application/ports/i_cli_runtime_factory.hpp
#ifndef APPLICATION_PORTS_I_CLI_RUNTIME_FACTORY_H_
#define APPLICATION_PORTS_I_CLI_RUNTIME_FACTORY_H_

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>

#include "application/dto/cli_config.hpp"
#include "application/use_cases/i_tracer_core_api.hpp"

namespace tracer_core::application::ports {

struct CliRuntimeRequest {
  std::filesystem::path executable_path;
  std::string command_name;
  std::optional<std::filesystem::path> db_override;
  std::optional<std::filesystem::path> output_override;
};

struct CliRuntime {
  std::shared_ptr<ITracerCoreApi> core_api;
  tracer_core::application::dto::CliConfig cli_config;
  std::shared_ptr<void> runtime_state;
};

enum class CliRuntimeValidationFailure : std::uint8_t {
  kNone = 0,
  kRuntimeDependencyMissing = 1,
  kConfigurationError = 2,
  kIoError = 3,
  kInvalidArguments = 4,
  kUnknownError = 5,
};

class ICliRuntimeFactory {
 public:
  virtual ~ICliRuntimeFactory() = default;

  [[nodiscard]] virtual auto ValidateEnvironment(
      const std::filesystem::path& executable_path, bool is_help_mode) const
      -> bool = 0;

  [[nodiscard]] virtual auto GetLastValidationFailure() const
      -> CliRuntimeValidationFailure = 0;

  [[nodiscard]] virtual auto BuildRuntime(
      const CliRuntimeRequest& request) const -> CliRuntime = 0;
};

// Default runtime factory provider. Implemented in infrastructure.
auto CreateCliRuntimeFactory() -> std::shared_ptr<ICliRuntimeFactory>;

}  // namespace tracer_core::application::ports

#endif  // APPLICATION_PORTS_I_CLI_RUNTIME_FACTORY_H_
