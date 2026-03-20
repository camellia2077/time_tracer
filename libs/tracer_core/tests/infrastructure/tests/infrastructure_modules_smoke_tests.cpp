#include "infrastructure/tests/modules_smoke/support.hpp"

auto main() -> int {
  const int logging_status = RunInfrastructureModuleLoggingPlatformConfigSmoke();
  if (logging_status != 0) {
    return logging_status;
  }

  const int crypto_exchange_status =
      RunInfrastructureModuleCryptoExchangeSmoke();
  if (crypto_exchange_status != 0) {
    return crypto_exchange_status;
  }

  const int query_stats_status =
      RunInfrastructureModuleQueryStatsRepositorySmoke();
  if (query_stats_status != 0) {
    return query_stats_status;
  }

  const int query_internal_status =
      RunInfrastructureModuleQueryInternalOrchestratorsSmoke();
  if (query_internal_status != 0) {
    return query_internal_status;
  }

  const int persistence_status = RunInfrastructureModulePersistenceSmoke();
  if (persistence_status != 0) {
    return persistence_status;
  }

  return RunInfrastructureModuleReportsSmoke();
}
