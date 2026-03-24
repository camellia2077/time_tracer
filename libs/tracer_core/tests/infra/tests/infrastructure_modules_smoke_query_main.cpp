#include "infra/tests/modules_smoke/query.hpp"

auto main() -> int {
  const int stats_status = RunInfrastructureModuleQueryStatsRepositorySmoke();
  if (stats_status != 0) {
    return stats_status;
  }
  return RunInfrastructureModuleQueryInternalOrchestratorsSmoke();
}
