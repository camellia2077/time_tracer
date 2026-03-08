#include <iostream>

#include "transport_runtime_codec_test_common.hpp"

auto main() -> int {
  using namespace tracer_transport_runtime_codec_tests;

  int failures = 0;
  RunDecodeRequestTests(failures);
  RunDecodeResponseTests(failures);
  RunEncodeTests(failures);

  if (failures == 0) {
    std::cout << "[PASS] tracer_transport_runtime_codec_tests\n";
    return 0;
  }

  std::cerr << "[FAIL] tracer_transport_runtime_codec_tests failures: "
            << failures << '\n';
  return 1;
}
