#include "infrastructure/tests/legacy_headers_compat/support.hpp"

auto main() -> int {
  int failures = 0;
  TestLegacyLoggingHeaders(failures);
  TestLegacyPlatformHeaders(failures);
  TestLegacyConfigHeaders(failures);
  TestLegacyQueryDataStatsHeaders(failures);
  TestLegacyQueryDataRepositoryAndRendererHeaders(failures);
  TestLegacyQueryDataInternalHeaders(failures);
  TestLegacyQueryDataOrchestratorHeaders(failures);
  TestLegacyPersistenceWriteHeaders(failures);
  TestLegacyPersistenceRuntimeHeaders(failures);
  TestLegacyReportsExportHeaders(failures);
  TestLegacyReportsDtoHeaders(failures);
  TestLegacyReportsQueryingHeaders(failures);
  TestLegacyReportsDataQueryingHeaders(failures);

  if (failures == 0) {
    std::cout << "[PASS] tracer_core_infrastructure_legacy_headers_compat_tests\n";
    return 0;
  }

  std::cerr
      << "[FAIL] tracer_core_infrastructure_legacy_headers_compat_tests failures: "
      << failures << '\n';
  return 1;
}
