// api/c_api/tracer_core_c_api_internal.cpp
import tracer.core.application.use_cases.interface;

#include "api/c_api/tracer_core_c_api_internal.hpp"

#include <mutex>
#include <stdexcept>
#include <utility>

#include "application/dto/query_responses.hpp"
#include "application/dto/shared_envelopes.hpp"
#include "api/c_api/c_api_parse_bridge.hpp"
#include "tracer/transport/runtime_codec.hpp"

#ifndef TT_ENABLE_PROCESSED_JSON_IO
#define TT_ENABLE_PROCESSED_JSON_IO 1
#endif

#ifndef TT_REPORT_ENABLE_LATEX
#define TT_REPORT_ENABLE_LATEX 1
#endif

#ifndef TT_REPORT_ENABLE_TYPST
#define TT_REPORT_ENABLE_TYPST 1
#endif

namespace tracer_core::core::c_api::internal {

namespace tt_transport = tracer::transport;
using tracer::core::application::use_cases::ITracerCoreRuntime;

thread_local std::string g_last_error;
thread_local std::string g_last_response;

namespace {

std::mutex g_crypto_progress_callback_mutex;
CryptoProgressCallbackRegistration g_crypto_progress_callback_registration{};

}  // namespace

void SetCryptoProgressCallbackRegistration(TtCoreCryptoProgressCallback callback,
                                           void* user_data) {
  std::scoped_lock lock(g_crypto_progress_callback_mutex);
  g_crypto_progress_callback_registration.callback = callback;
  g_crypto_progress_callback_registration.user_data = user_data;
}

[[nodiscard]] auto GetCryptoProgressCallbackRegistration()
    -> CryptoProgressCallbackRegistration {
  std::scoped_lock lock(g_crypto_progress_callback_mutex);
  return g_crypto_progress_callback_registration;
}

namespace {

constexpr std::string_view kRuntimeGenericErrorCode = "runtime.generic_error";
constexpr std::string_view kRuntimeErrorCategory = "runtime";
constexpr std::string_view kErrorHintCheckMessage =
    "Inspect `error_message` for detailed failure reason.";

struct ErrorContractBuildOptions {
  std::string_view error_code = kRuntimeGenericErrorCode;
  std::string_view error_category = kRuntimeErrorCategory;
  std::vector<std::string> hints;
};

[[nodiscard]] auto BuildErrorContract(
    bool is_ok, std::string_view error_message,
    ErrorContractBuildOptions options = {}) -> tt_transport::ErrorContractPayload {
  tt_transport::ErrorContractPayload contract{};
  if (is_ok) {
    return contract;
  }
  contract.error_code = std::string(options.error_code);
  contract.error_category = std::string(options.error_category);
  if (options.hints.empty() && !error_message.empty()) {
    options.hints.emplace_back(kErrorHintCheckMessage);
  }
  contract.hints = std::move(options.hints);
  return contract;
}

[[nodiscard]] auto BuildAckResponse(bool is_ok,
                                    const std::string& error_message) -> const
    char* {
  g_last_response =
      tt_transport::EncodeIngestResponse(tt_transport::IngestResponsePayload{
          .ok = is_ok,
          .error_message = error_message,
          .error_contract = BuildErrorContract(is_ok, error_message),
      });
  return g_last_response.c_str();
}

auto ConvertTreeNode(const ProjectTreeNode& node)
    -> tt_transport::ProjectTreeNodePayload {
  tt_transport::ProjectTreeNodePayload out{};
  out.name = node.name;
  if (!node.path.empty()) {
    out.path = node.path;
  }
  out.duration_seconds = node.duration_seconds;
  out.children.reserve(node.children.size());
  for (const auto& child : node.children) {
    out.children.push_back(ConvertTreeNode(child));
  }
  return out;
}

}  // namespace

#include "api/c_api/internal/tracer_core_c_api_internal_impl.inc"

}  // namespace tracer_core::core::c_api::internal

