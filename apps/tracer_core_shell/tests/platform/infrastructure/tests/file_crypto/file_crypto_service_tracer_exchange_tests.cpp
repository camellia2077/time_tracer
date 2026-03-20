// infrastructure/tests/file_crypto/file_crypto_service_tracer_exchange_tests.cpp
#include "infrastructure/tests/file_crypto/file_crypto_service_tracer_exchange_test_support.hpp"

namespace android_runtime_tests {

auto RunFileCryptoTracerExchangeTests(int& failures) -> void {
  RunFileCryptoTracerExchangePackageTests(failures);
  RunFileCryptoTracerExchangeExportTests(failures);
  RunFileCryptoTracerExchangeImportTests(failures);
}

}  // namespace android_runtime_tests
