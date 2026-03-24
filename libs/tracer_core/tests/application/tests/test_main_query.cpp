// application/tests/test_main_query.cpp
#include "application/tests/modules/query_tests.hpp"
#include "application/tests/support/test_support.hpp"

auto main() -> int {
  tracer_core::application::tests::TestState state;

  tracer_core::application::tests::RunDataQueryTests(state);

  if (state.failures == 0) {
    std::cout << "[PASS] time_tracker_query_api_tests" << '\n';
    return 0;
  }

  std::cerr << "[FAIL] time_tracker_query_api_tests failures: " << state.failures
            << '\n';
  return 1;
}
