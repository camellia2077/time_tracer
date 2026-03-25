#include <iostream>

#include "infrastructure/tests/android_runtime/android_runtime_test_common.hpp"

auto main() -> int {
  int failures = 0;
  android_runtime_tests::RunFileCryptoTracerExchangeTests(failures);

  if (failures == 0) {
    std::cout << "[PASS] time_tracker_exchange_runtime_tests\n";
    return 0;
  }

  std::cerr << "[FAIL] time_tracker_exchange_runtime_tests failures: "
            << failures << '\n';
  return 1;
}
