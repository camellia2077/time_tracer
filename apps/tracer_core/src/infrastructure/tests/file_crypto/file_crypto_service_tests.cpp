// infrastructure/tests/file_crypto/file_crypto_service_tests.cpp
#include <iostream>

#include "infrastructure/tests/file_crypto/file_crypto_service_test_internal.hpp"

namespace android_runtime_tests {

auto RunFileCryptoServiceTests(int& failures) -> void {
#if defined(TT_HAS_LIBSODIUM) && TT_HAS_LIBSODIUM && defined(TT_HAS_ZSTD) && \
    TT_HAS_ZSTD
  RunFileCryptoRoundtripTests(failures);
  RunFileCryptoFailureTests(failures);
  RunFileCryptoProgressTests(failures);
  RunFileCryptoInteropTests(failures);
#else
  (void)failures;
  std::cout << "[INFO] Skip file crypto tests: libsodium/zstd backend is not "
               "enabled.\n";
#endif
}

}  // namespace android_runtime_tests
