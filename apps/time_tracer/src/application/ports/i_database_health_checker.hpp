// application/ports/i_database_health_checker.hpp
#ifndef APPLICATION_PORTS_I_DATABASE_HEALTH_CHECKER_H_
#define APPLICATION_PORTS_I_DATABASE_HEALTH_CHECKER_H_

#include <string>

namespace time_tracer::application::ports {

struct DatabaseHealthCheckResult {
  bool ok = false;
  std::string message;
};

class IDatabaseHealthChecker {
 public:
  virtual ~IDatabaseHealthChecker() = default;

  virtual auto CheckReady() -> DatabaseHealthCheckResult = 0;
};

}  // namespace time_tracer::application::ports

#endif  // APPLICATION_PORTS_I_DATABASE_HEALTH_CHECKER_H_
