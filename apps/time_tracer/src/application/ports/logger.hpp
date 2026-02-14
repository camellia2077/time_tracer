// application/ports/logger.hpp
#ifndef APPLICATION_PORTS_LOGGER_H_
#define APPLICATION_PORTS_LOGGER_H_

#include <memory>
#include <string_view>

namespace time_tracer::application::ports {

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

}  // namespace time_tracer::application::ports

#endif  // APPLICATION_PORTS_LOGGER_H_
