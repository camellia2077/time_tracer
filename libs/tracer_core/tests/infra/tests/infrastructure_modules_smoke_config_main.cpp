#include "infra/tests/modules_smoke/config.hpp"

auto main() -> int {
  const int config_smoke = RunInfrastructureModuleLoggingPlatformConfigSmoke();
  if (config_smoke != 0) {
    return config_smoke;
  }
  return RunActivityHierarchyCharacterizationTests();
}
