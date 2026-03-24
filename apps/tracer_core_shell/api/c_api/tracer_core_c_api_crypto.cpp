#include <algorithm>
#include <cctype>
#include <exception>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "api/c_api/tracer_core_c_api.h"
#include "api/c_api/tracer_core_c_api_internal.hpp"
#include "application/dto/exchange_requests.hpp"
#include "application/dto/exchange_responses.hpp"
#include "application/dto/shared_envelopes.hpp"
#include "application/aggregate_runtime/i_tracer_core_runtime.hpp"
#include "host/crypto_progress_bridge.hpp"
#include "host/tracer_exchange_inspect_formatter.hpp"
#include "nlohmann/json.hpp"

using tracer::core::application::use_cases::ITracerCoreRuntime;

using tracer_core::core::c_api::internal::BuildFailureResponse;
using tracer_core::core::c_api::internal::BuildTextResponse;
using tracer_core::core::c_api::internal::ClearLastError;
using tracer_core::core::c_api::internal::ParseDateCheckMode;
using tracer_core::core::c_api::internal::RequireRuntime;
using tracer_core::core::c_api::internal::ToRequestJsonView;

namespace {

namespace fs = std::filesystem;
namespace app_dto = tracer_core::core::dto;
using nlohmann::json;

#include "api/c_api/internal/tracer_core_c_api_crypto_parse_impl.inc"

// Build user-facing TextOutput envelopes from exchange results.
#include "api/c_api/internal/tracer_core_c_api_crypto_response_impl.inc"

// Construct exchange DTOs, attach progress bridge, and dispatch to runtime.
#include "api/c_api/internal/tracer_core_c_api_crypto_dispatch_impl.inc"

// Wrap TextOutput endpoints into stable C ABI responses.
#include "api/c_api/internal/tracer_core_c_api_crypto_entry_impl.inc"

}  // namespace

extern "C" TT_CORE_API auto tracer_core_runtime_crypto_encrypt_json(
    TtCoreRuntimeHandle* handle, const char* request_json) -> const char* {
  return ExecuteCryptoTextEndpoint(
      "tracer_core_runtime_crypto_encrypt_json failed unexpectedly.",
      [&]() {
        const CryptoEncryptExecution execution =
            DispatchCryptoEncrypt(handle,
                                  ParseCryptoEncryptArgs(
                                      ToRequestJsonView(request_json)));
        return BuildEncryptTextOutput(execution.result, execution.request);
      });
}

extern "C" TT_CORE_API auto tracer_core_runtime_crypto_decrypt_json(
    TtCoreRuntimeHandle* handle, const char* request_json) -> const char* {
  return ExecuteCryptoTextEndpoint(
      "tracer_core_runtime_crypto_decrypt_json failed unexpectedly.",
      [&]() {
        const CryptoDecryptExecution execution =
            DispatchCryptoDecrypt(handle,
                                  ParseCryptoDecryptArgs(
                                      ToRequestJsonView(request_json)));
        return BuildImportTextOutput(execution.result, execution.request);
      });
}

extern "C" TT_CORE_API auto tracer_core_runtime_crypto_inspect_json(
    TtCoreRuntimeHandle* handle, const char* request_json) -> const char* {
  return ExecuteCryptoTextEndpoint(
      "tracer_core_runtime_crypto_inspect_json failed unexpectedly.",
      [&]() {
        const CryptoInspectExecution execution =
            DispatchCryptoInspect(handle,
                                  ParseCryptoInspectArgs(
                                      ToRequestJsonView(request_json)));
        return BuildInspectTextOutput(execution.result);
      });
}
