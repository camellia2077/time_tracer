// application/tests/test_main_aggregate_runtime.cpp
#include "application/tests/modules/aggregate_runtime_tests.hpp"
#include "application/tests/support/test_support.hpp"

auto main() -> int {
  tracer_core::application::tests::TestState state;

  tracer_core::application::tests::RunTracerCoreRuntimeTests(state);

  if (state.failures == 0) {
    std::cout << "[PASS] time_tracker_aggregate_runtime_tests" << '\n';
    return 0;
  }

  std::cerr << "[FAIL] time_tracker_aggregate_runtime_tests failures: "
            << state.failures << '\n';
  return 1;
}
