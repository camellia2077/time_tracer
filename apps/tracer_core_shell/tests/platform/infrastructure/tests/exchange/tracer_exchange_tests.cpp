// infrastructure/tests/exchange/tracer_exchange_tests.cpp
#include "infrastructure/tests/exchange/tracer_exchange_test_support.hpp"

namespace android_runtime_tests {

auto RunTracerExchangeTests(int& failures) -> void {
  RunTracerExchangePackageTests(failures);
}

}  // namespace android_runtime_tests
