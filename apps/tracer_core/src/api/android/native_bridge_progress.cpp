// api/android/native_bridge_progress.cpp
#include <algorithm>
#include <array>
#include <cctype>
#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <nlohmann/json.hpp>
#include <optional>
#include <ranges>
#include <string>

#include "api/android/native_bridge_crypto_helpers.hpp"
#include "api/android/native_bridge_internal.hpp"

namespace tracer_core::api::android::bridge_internal {

using nlohmann::json;
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

[[nodiscard]] auto ToOperationWireValue(
    file_crypto::FileCryptoOperation operation) -> std::string_view {
  switch (operation) {
    case file_crypto::FileCryptoOperation::kEncrypt:
      return "encrypt";
    case file_crypto::FileCryptoOperation::kDecrypt:
      return "decrypt";
  }
  return "unknown";
}

[[nodiscard]] auto ToPhaseWireValue(file_crypto::FileCryptoPhase phase)
    -> std::string_view {
  switch (phase) {
    case file_crypto::FileCryptoPhase::kScan:
      return "scan";
    case file_crypto::FileCryptoPhase::kReadInput:
      return "read_input";
    case file_crypto::FileCryptoPhase::kCompress:
      return "compress";
    case file_crypto::FileCryptoPhase::kDeriveKey:
      return "derive_key";
    case file_crypto::FileCryptoPhase::kEncrypt:
      return "encrypt";
    case file_crypto::FileCryptoPhase::kDecrypt:
      return "decrypt";
    case file_crypto::FileCryptoPhase::kDecompress:
      return "decompress";
    case file_crypto::FileCryptoPhase::kWriteOutput:
      return "write_output";
    case file_crypto::FileCryptoPhase::kCompleted:
      return "completed";
    case file_crypto::FileCryptoPhase::kCancelled:
      return "cancelled";
    case file_crypto::FileCryptoPhase::kFailed:
      return "failed";
  }
  return "unknown";
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
      json{
          {"operation", ToOperationWireValue(snapshot.operation)},
          {"phase", ToPhaseWireValue(snapshot.phase)},
          {"current_group_label", snapshot.current_group_label},
          {"group_index", snapshot.group_index},
          {"group_count", snapshot.group_count},
          {"file_index_in_group", snapshot.file_index_in_group},
          {"file_count_in_group", snapshot.file_count_in_group},
          {"current_file_index", snapshot.current_file_index},
          {"total_files", snapshot.total_files},
          {"current_file_done_bytes", snapshot.current_file_done_bytes},
          {"current_file_total_bytes", snapshot.current_file_total_bytes},
          {"overall_done_bytes", snapshot.overall_done_bytes},
          {"overall_total_bytes", snapshot.overall_total_bytes},
          {"speed_bytes_per_sec", snapshot.speed_bytes_per_sec},
          {"remaining_bytes", snapshot.remaining_bytes},
          {"eta_seconds", snapshot.eta_seconds},
          {"current_input_path", snapshot.current_input_path.string()},
          {"current_output_path", snapshot.current_output_path.string()},
          {"input_root_path", snapshot.input_root_path.string()},
          {"output_root_path", snapshot.output_root_path.string()},
      }
          .dump();

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
