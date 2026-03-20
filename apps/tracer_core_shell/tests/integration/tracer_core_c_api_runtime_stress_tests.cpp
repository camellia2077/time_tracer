// tests/integration/tracer_core_c_api_runtime_stress_tests.cpp
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

#include "tests/integration/tracer_core_c_api_stability_internal.hpp"

namespace tracer_core_c_api_stability_internal {
namespace {

struct CallbackProbeState {
  std::atomic<int> kLogCount{0};
  std::atomic<int> kDiagnosticsCount{0};
  std::atomic<int> kCryptoProgressCount{0};
};

extern "C" void CaptureLogCallback(TtCoreLogSeverity /*severity*/,
                                   const char* utf8_message,
                                   void* user_data) {
  if (user_data == nullptr) {
    return;
  }
  auto* state = static_cast<CallbackProbeState*>(user_data);
  if (utf8_message != nullptr && utf8_message[0] != '\0') {
    state->kLogCount.fetch_add(1, std::memory_order_relaxed);
  }
}

extern "C" void CaptureDiagnosticsCallback(
    TtCoreDiagnosticSeverity /*severity*/, const char* utf8_message,
    void* user_data) {
  if (user_data == nullptr) {
    return;
  }
  auto* state = static_cast<CallbackProbeState*>(user_data);
  if (utf8_message != nullptr && utf8_message[0] != '\0') {
    state->kDiagnosticsCount.fetch_add(1, std::memory_order_relaxed);
  }
}

extern "C" void CaptureCryptoProgressCallback(const char* utf8_progress_json,
                                              void* user_data) {
  if (user_data == nullptr || utf8_progress_json == nullptr ||
      utf8_progress_json[0] == '\0') {
    return;
  }

  try {
    const json kPayload = json::parse(utf8_progress_json);
    if (!kPayload.is_object()) {
      return;
    }
    auto* state = static_cast<CallbackProbeState*>(user_data);
    state->kCryptoProgressCount.fetch_add(1, std::memory_order_relaxed);
  } catch (...) {
    // Keep callback resilient; test only counts valid JSON payloads.
  }
}

}  // namespace

void RunConcurrentChecks(const CoreApiFns& api,
                         const RuntimePathBundle& paths) {
  static_cast<void>(paths.db_path);
  static_cast<void>(paths.temp_root);
  static_cast<void>(paths.converter_config);
  constexpr int kThreadCount = 8;
  constexpr int kIterationsPerThread = 40;

  std::mutex error_mutex;
  std::string first_error;
  std::atomic<bool> failed{false};

  auto worker = [&](int thread_index) -> void {
    try {
      static_cast<void>(thread_index);
      for (int iteration = 0; iteration < kIterationsPerThread; ++iteration) {
        const int kPingResult = api.ping();
        Require(kPingResult == TT_CORE_STATUS_OK,
                "concurrency ping should succeed");
        const char* version = api.get_version();
        Require(version != nullptr && version[0] != '\0',
                "concurrency version should be non-empty");
        RequireNotOk(api.runtime_query(
                         nullptr, json{{"action", "years"}}.dump().c_str()),
                     "concurrency invalid runtime query should fail");
      }
    } catch (const std::exception& error) {
      if (!failed.exchange(true)) {
        std::scoped_lock lock(error_mutex);
        first_error = error.what();
      }
    }
  };

  std::vector<std::thread> threads;
  threads.reserve(kThreadCount);
  for (int i = 0; i < kThreadCount; ++i) {
    threads.emplace_back(worker, i);
  }
  for (auto& thread : threads) {
    thread.join();
  }

  if (failed.load()) {
    throw std::runtime_error("Concurrent checks failed: " + first_error);
  }
}

void RunCreateDestroyChurn(const CoreApiFns& api,
                           const RuntimePathBundle& paths) {
  constexpr int kChurnIterations = 6;
  for (int i = 0; i < kChurnIterations; ++i) {
    const fs::path kOutputRoot =
        paths.temp_root / "churn" / ("iter_" + std::to_string(i));
    auto runtime =
        CreateRuntime(api, paths.db_path, kOutputRoot, paths.converter_config);
    Require(runtime.Get() != nullptr,
            "create-destroy churn runtime should be created");
  }
}

void RunCallbackBridgeChecks(const CoreApiFns& api,
                             TtCoreRuntimeHandle* runtime,
                             const fs::path& input_root) {
  CallbackProbeState callback_state{};
  api.set_log_callback(&CaptureLogCallback, &callback_state);
  api.set_diagnostics_callback(&CaptureDiagnosticsCallback, &callback_state);
  api.set_crypto_progress_callback(&CaptureCryptoProgressCallback,
                                   &callback_state);

  const std::string kIngestRequest = json{{"input_path", input_root.string()},
                                          {"date_check_mode", "none"},
                                          {"save_processed_output", false}}
                                         .dump();
  RequireOk(api.runtime_ingest(runtime, kIngestRequest.c_str()),
            "callback ingest");

  const fs::path kCryptoInput = input_root;
  Require(fs::exists(kCryptoInput),
          "callback crypto input directory missing: test/data");
  const fs::path kCryptoOutput = input_root / ".." / "output" /
                                 "tracer_core_c_api_stability" / "callback" /
                                 "test-data.tracer";
  std::error_code io_error;
  fs::create_directories(fs::absolute(kCryptoOutput).parent_path(), io_error);
  Require(!io_error,
          "callback crypto output directory creation failed: " +
              io_error.message());
  const std::string kCryptoEncryptRequest =
      json{{"input_path", fs::absolute(kCryptoInput).string()},
           {"output_path", fs::absolute(kCryptoOutput).string()},
           {"passphrase", "phase6-progress-callback"},
           {"security_level", "interactive"},
           {"date_check_mode", "none"}}
          .dump();
  RequireOk(api.runtime_crypto_encrypt(runtime, kCryptoEncryptRequest.c_str()),
            "callback crypto encrypt");

  api.set_log_callback(nullptr, nullptr);
  api.set_diagnostics_callback(nullptr, nullptr);
  api.set_crypto_progress_callback(nullptr, nullptr);

  const int kLogCount = callback_state.kLogCount.load(std::memory_order_relaxed);
  const int kDiagnosticsCount =
      callback_state.kDiagnosticsCount.load(std::memory_order_relaxed);
  const int kCryptoProgressCount =
      callback_state.kCryptoProgressCount.load(std::memory_order_relaxed);
  Require(kLogCount > 0 || kDiagnosticsCount > 0,
          "runtime callbacks should receive at least one log or diagnostics "
          "message during ingest");
  Require(kCryptoProgressCount > 0,
          "runtime crypto progress callback should receive at least one "
          "valid JSON progress payload during encrypt");
}

}  // namespace tracer_core_c_api_stability_internal
