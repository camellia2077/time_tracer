// api/android/native_bridge_progress.cpp
#include <algorithm>
#include <array>
#include <cctype>
#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <optional>
#include <ranges>
#include <string>

#include "api/android/native_bridge_crypto_helpers.hpp"
#include "api/android/native_bridge_internal.hpp"
#include "api/shared/crypto_progress_json.hpp"

namespace tracer_core::api::android::bridge_internal {

namespace file_crypto = tracer_core::infrastructure::crypto;

namespace {

struct CryptoProgressThrottleConfig {
  std::chrono::milliseconds min_interval{40};
  std::uint64_t min_bytes_delta = 4U * 1024U;
};

auto ToLowerAscii(std::string value) -> std::string {
  std::ranges::transform(value, value.begin(), [](unsigned char ch) -> char {
    return static_cast<char>(std::tolower(ch));
  });
  return value;
}

auto TryReadEnvU64(const char* key) -> std::optional<std::uint64_t> {
  if (key == nullptr || key[0] == '\0') {
    return std::nullopt;
  }
  const char* raw_value = std::getenv(key);
  if (raw_value == nullptr || raw_value[0] == '\0') {
    return std::nullopt;
  }

  errno = 0;
  char* end = nullptr;
  const unsigned long long parsed = std::strtoull(raw_value, &end, 10);
  if (errno != 0 || end == nullptr || *end != '\0') {
    return std::nullopt;
  }
  return static_cast<std::uint64_t>(parsed);
}

auto ResolveCryptoProgressThrottleConfig()
    -> const CryptoProgressThrottleConfig& {
  static const CryptoProgressThrottleConfig config = [] {
    constexpr std::uint64_t kDefaultIntervalMs = 40;
    constexpr std::uint64_t kDefaultBytesDelta = 4U * 1024U;
    constexpr std::uint64_t kMinIntervalMs = 0;
    constexpr std::uint64_t kMaxIntervalMs = 1000;
    constexpr std::uint64_t kMinBytesDelta = 0;
    constexpr std::uint64_t kMaxBytesDelta = 4U * 1024U * 1024U;

    const std::uint64_t interval_ms =
        std::clamp(TryReadEnvU64("TT_CRYPTO_PROGRESS_MIN_INTERVAL_MS")
                       .value_or(kDefaultIntervalMs),
                   kMinIntervalMs, kMaxIntervalMs);
    const std::uint64_t bytes_delta =
        std::clamp(TryReadEnvU64("TT_CRYPTO_PROGRESS_MIN_BYTES_DELTA")
                       .value_or(kDefaultBytesDelta),
                   kMinBytesDelta, kMaxBytesDelta);

    CryptoProgressThrottleConfig resolved{};
    resolved.min_interval = std::chrono::milliseconds(interval_ms);
    resolved.min_bytes_delta = bytes_delta;
    return resolved;
  }();
  return config;
}

auto EmitCryptoProgress(JNIEnv* env,
                        const file_crypto::FileCryptoProgressSnapshot& snapshot)
    -> void {
  if (env == nullptr) {
    return;
  }

  constexpr std::array<const char*, 4> kBridgeClassCandidates = {
      "com/example/tracer/NativeBridge",
      "com/time_tracer/core/NativeBridge",
      "com/timetracer/NativeBridge",
      "NativeBridge",
  };

  jclass bridge_class = nullptr;
  for (const char* class_name : kBridgeClassCandidates) {
    bridge_class = env->FindClass(class_name);
    if (bridge_class != nullptr) {
      break;
    }
    env->ExceptionClear();
  }
  if (bridge_class == nullptr) {
    return;
  }

  jmethodID on_progress = env->GetStaticMethodID(
      bridge_class, "onCryptoProgressJson", "(Ljava/lang/String;)V");
  if (on_progress == nullptr) {
    env->ExceptionClear();
    env->DeleteLocalRef(bridge_class);
    return;
  }

  const std::string progress_json =
      tracer_core::api::shared::crypto_progress::BuildProgressSnapshotJson(
          snapshot);

  jstring payload_jstring = ToJString(env, progress_json);
  env->CallStaticVoidMethod(bridge_class, on_progress, payload_jstring);
  if (env->ExceptionCheck()) {
    env->ExceptionClear();
  }
  env->DeleteLocalRef(payload_jstring);
  env->DeleteLocalRef(bridge_class);
}

}  // namespace

auto ParseCryptoSecurityLevel(std::string_view value)
    -> std::optional<file_crypto::FileCryptoSecurityLevel> {
  const std::string normalized_value = ToLowerAscii(std::string(value));
  if (normalized_value.empty() || normalized_value == "interactive") {
    return file_crypto::FileCryptoSecurityLevel::kInteractive;
  }
  if (normalized_value == "moderate") {
    return file_crypto::FileCryptoSecurityLevel::kModerate;
  }
  if (normalized_value == "high") {
    return file_crypto::FileCryptoSecurityLevel::kHigh;
  }
  return std::nullopt;
}

auto BuildCryptoOptions(JNIEnv* env) -> file_crypto::FileCryptoOptions {
  const auto& throttle = ResolveCryptoProgressThrottleConfig();
  file_crypto::FileCryptoOptions options{};
  options.progress_min_interval = throttle.min_interval;
  options.progress_min_bytes_delta = throttle.min_bytes_delta;
  options.progress_callback =
      [env](const file_crypto::FileCryptoProgressSnapshot& snapshot) {
        EmitCryptoProgress(env, snapshot);
        return file_crypto::FileCryptoControl::kContinue;
      };
  return options;
}

}  // namespace tracer_core::api::android::bridge_internal
