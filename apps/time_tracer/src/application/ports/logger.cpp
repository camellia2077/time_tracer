// application/ports/logger.cpp
#include "application/ports/logger.hpp"

#include <memory>
#include <mutex>

namespace time_tracer::application::ports {
namespace {

class NullLogger final : public ILogger {
 public:
  auto Log(LogSeverity /*severity*/, std::string_view /*message*/)
      -> void override {}
};

std::mutex g_logger_mutex;
std::shared_ptr<ILogger> g_logger = std::make_shared<NullLogger>();

}  // namespace

auto SetLogger(std::shared_ptr<ILogger> logger) -> void {
  std::scoped_lock lock(g_logger_mutex);
  if (logger) {
    g_logger = std::move(logger);
  } else {
    g_logger = std::make_shared<NullLogger>();
  }
}

auto GetLogger() -> std::shared_ptr<ILogger> {
  std::scoped_lock lock(g_logger_mutex);
  return g_logger;
}

auto LogInfo(std::string_view message) -> void {
  GetLogger()->Log(LogSeverity::kInfo, message);
}

auto LogWarn(std::string_view message) -> void {
  GetLogger()->Log(LogSeverity::kWarn, message);
}

auto LogError(std::string_view message) -> void {
  GetLogger()->Log(LogSeverity::kError, message);
}

}  // namespace time_tracer::application::ports
