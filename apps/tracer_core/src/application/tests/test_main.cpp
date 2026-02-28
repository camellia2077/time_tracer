// application/tests/test_main.cpp
#include "application/tests/modules/test_modules.hpp"
#include "application/tests/support/test_support.hpp"
// 测试入口文件，天然小文件,不建议合并
auto main() -> int {
  tracer_core::application::tests::TestState state;

  tracer_core::application::tests::RunConvertIngestValidateTests(state);
  tracer_core::application::tests::RunReportTests(state);
  tracer_core::application::tests::RunDataQueryTests(state);
  tracer_core::application::tests::RunImportServiceTests(state);

  if (state.failures == 0) {
    std::cout << "[PASS] time_tracker_core_api_tests" << '\n';
    return 0;
  }

  std::cerr << "[FAIL] time_tracker_core_api_tests failures: " << state.failures
            << '\n';
  return 1;
}
