// application/tests/test_main_exchange.cpp
#include "application/tests/modules/exchange_tests.hpp"
#include "application/tests/support/test_support.hpp"

auto main() -> int {
  tracer_core::application::tests::TestState state;

  tracer_core::application::tests::RunTracerExchangeTests(state);

  if (state.failures == 0) {
    std::cout << "[PASS] time_tracker_exchange_api_tests" << '\n';
    return 0;
  }

  std::cerr << "[FAIL] time_tracker_exchange_api_tests failures: "
            << state.failures << '\n';
  return 1;
}
