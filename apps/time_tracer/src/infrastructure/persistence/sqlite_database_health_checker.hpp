// infrastructure/persistence/sqlite_database_health_checker.hpp
#ifndef INFRASTRUCTURE_PERSISTENCE_SQLITE_DATABASE_HEALTH_CHECKER_H_
#define INFRASTRUCTURE_PERSISTENCE_SQLITE_DATABASE_HEALTH_CHECKER_H_

#include <string>

#include "application/ports/i_database_health_checker.hpp"

namespace infrastructure::persistence {

class SqliteDatabaseHealthChecker final
    : public time_tracer::application::ports::IDatabaseHealthChecker {
 public:
  explicit SqliteDatabaseHealthChecker(std::string db_path);

  auto CheckReady()
      -> time_tracer::application::ports::DatabaseHealthCheckResult override;

 private:
  std::string db_path_;
};

}  // namespace infrastructure::persistence

#endif  // INFRASTRUCTURE_PERSISTENCE_SQLITE_DATABASE_HEALTH_CHECKER_H_
