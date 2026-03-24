// application/runtime_bridge/logger.hpp
#ifndef APPLICATION_RUNTIME_BRIDGE_LOGGER_HPP_
#define APPLICATION_RUNTIME_BRIDGE_LOGGER_HPP_

#include <memory>
#include <string_view>

namespace tracer_core::application::runtime_bridge {

// Shell/runtime bridge logging contract. This is not a capability-owned port.
enum class LogSeverity {
  kInfo,
  kWarn,
  kError,
};

class ILogger {
 public:
  virtual ~ILogger() = default;
  virtual auto Log(LogSeverity severity, std::string_view message) -> void = 0;
};

auto SetLogger(std::shared_ptr<ILogger> logger) -> void;
auto GetLogger() -> std::shared_ptr<ILogger>;

auto LogInfo(std::string_view message) -> void;
auto LogWarn(std::string_view message) -> void;
auto LogError(std::string_view message) -> void;

}  // namespace tracer_core::application::runtime_bridge

#endif  // APPLICATION_RUNTIME_BRIDGE_LOGGER_HPP_
