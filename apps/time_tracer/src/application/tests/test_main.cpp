// application/tests/test_main.cpp
#include "application/tests/modules/test_modules.hpp"
#include "application/tests/support/test_support.hpp"
// 测试入口文件，天然小文件,不建议合并
auto main() -> int {
  time_tracer::application::tests::TestState state;

  time_tracer::application::tests::RunConvertIngestValidateTests(state);
  time_tracer::application::tests::RunReportTests(state);
  time_tracer::application::tests::RunDataQueryTests(state);
  time_tracer::application::tests::RunImportServiceTests(state);

  if (state.failures == 0) {
    std::cout << "[PASS] time_tracker_core_api_tests" << '\n';
    return 0;
  }

  std::cerr << "[FAIL] time_tracker_core_api_tests failures: " << state.failures
            << '\n';
  return 1;
}
