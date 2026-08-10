// application/tests/test_main_insights.cpp
#include "application/tests/modules/insights_tests.hpp"
#include "application/tests/support/test_support.hpp"

auto main() -> int {
  tracer_core::application::tests::TestState state;

  tracer_core::application::tests::RunInsightsTests(state);
  tracer_core::application::tests::RunInsightsSemanticsTests(state);

  if (state.failures == 0) {
    std::cout << "[PASS] time_tracker_insights_api_tests" << '\n';
    return 0;
  }

  std::cerr << "[FAIL] time_tracker_insights_api_tests failures: "
            << state.failures << '\n';
  return 1;
}
