#pragma once

#include <jni.h>

#include <optional>
#include <string_view>

#include "infrastructure/crypto/file_crypto_service.hpp"

namespace tracer_core::api::android::bridge_internal {

[[nodiscard]] auto ParseCryptoSecurityLevel(std::string_view value)
    -> std::optional<
        tracer_core::infrastructure::crypto::FileCryptoSecurityLevel>;

auto BuildCryptoOptions(JNIEnv* env)
    -> tracer_core::infrastructure::crypto::FileCryptoOptions;

}  // namespace tracer_core::api::android::bridge_internal
