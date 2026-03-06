// infrastructure/tests/file_crypto/file_crypto_test_main.cpp
#include <iostream>

#include "infrastructure/tests/android_runtime/android_runtime_test_common.hpp"

auto main() -> int {
  int failures = 0;
  android_runtime_tests::RunFileCryptoServiceTests(failures);

  if (failures == 0) {
    std::cout << "[PASS] time_tracker_file_crypto_tests\n";
    return 0;
  }

  std::cerr << "[FAIL] time_tracker_file_crypto_tests failures: " << failures
            << '\n';
  return 1;
}
