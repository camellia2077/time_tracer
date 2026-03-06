// infrastructure/crypto/internal/file_crypto_common.hpp
#ifndef INFRASTRUCTURE_CRYPTO_INTERNAL_FILE_CRYPTO_COMMON_HPP_
#define INFRASTRUCTURE_CRYPTO_INTERNAL_FILE_CRYPTO_COMMON_HPP_

#include <string>
#include <string_view>

#include "infrastructure/crypto/file_crypto_service.hpp"

namespace tracer_core::infrastructure::crypto::internal {

inline auto MakeError(FileCryptoError error, std::string_view message)
    -> FileCryptoResult {
  return {.error = error,
          .error_code = std::string(ToErrorCode(error)),
          .error_message = std::string(message)};
}

[[nodiscard]] inline auto IsCancelledError(const FileCryptoResult& result)
    -> bool {
  return result.error == FileCryptoError::kCancelled;
}

}  // namespace tracer_core::infrastructure::crypto::internal

#endif  // INFRASTRUCTURE_CRYPTO_INTERNAL_FILE_CRYPTO_COMMON_HPP_
