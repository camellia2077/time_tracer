// application/tests/test_main_reporting.cpp
#include "application/tests/modules/reporting_tests.hpp"
#include "application/tests/support/test_support.hpp"

auto main() -> int {
  tracer_core::application::tests::TestState state;

  tracer_core::application::tests::RunReportTests(state);
  tracer_core::application::tests::RunReportSemanticsTests(state);

  if (state.failures == 0) {
    std::cout << "[PASS] time_tracker_reporting_api_tests" << '\n';
    return 0;
  }

  std::cerr << "[FAIL] time_tracker_reporting_api_tests failures: "
            << state.failures << '\n';
  return 1;
}
