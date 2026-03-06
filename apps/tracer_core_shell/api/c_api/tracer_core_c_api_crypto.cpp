#include <algorithm>
#include <cctype>
#include <exception>
#include <filesystem>
#include <optional>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "api/c_api/tracer_core_c_api.h"
#include "api/c_api/tracer_core_c_api_internal.hpp"
#include "shared/crypto_progress_json.hpp"
#include "application/use_cases/i_tracer_core_api.hpp"
#include "infrastructure/crypto/file_crypto_service.hpp"
#include "nlohmann/json.hpp"

using tracer_core::core::c_api::internal::BuildFailureResponse;
using tracer_core::core::c_api::internal::BuildTextResponse;
using tracer_core::core::c_api::internal::ClearLastError;
using tracer_core::core::c_api::internal::ParseDateCheckMode;
using tracer_core::core::c_api::internal::RequireRuntime;
using tracer_core::core::c_api::internal::ToRequestJsonView;
using tracer_core::core::dto::TextOutput;
using tracer_core::core::dto::ValidateLogicRequest;
using tracer_core::core::dto::ValidateStructureRequest;

#include "api/c_api/internal/tracer_core_c_api_crypto_namespace.inc"


extern "C" TT_CORE_API auto tracer_core_runtime_crypto_encrypt_json(
    TtCoreRuntimeHandle* handle, const char* request_json) -> const char* {
  try {
    ClearLastError();
    ITracerCoreApi& runtime = RequireRuntime(handle);
    return BuildTextResponse(
        RunCryptoEncrypt(runtime, ToRequestJsonView(request_json)));
  } catch (const std::exception& error) {
    return BuildFailureResponse(error.what());
  } catch (...) {
    return BuildFailureResponse(
        "tracer_core_runtime_crypto_encrypt_json failed unexpectedly.");
  }
}

extern "C" TT_CORE_API auto tracer_core_runtime_crypto_decrypt_json(
    TtCoreRuntimeHandle* handle, const char* request_json) -> const char* {
  try {
    ClearLastError();
    (void)RequireRuntime(handle);
    return BuildTextResponse(RunCryptoDecrypt(ToRequestJsonView(request_json)));
  } catch (const std::exception& error) {
    return BuildFailureResponse(error.what());
  } catch (...) {
    return BuildFailureResponse(
        "tracer_core_runtime_crypto_decrypt_json failed unexpectedly.");
  }
}

extern "C" TT_CORE_API auto tracer_core_runtime_crypto_inspect_json(
    TtCoreRuntimeHandle* handle, const char* request_json) -> const char* {
  try {
    ClearLastError();
    (void)RequireRuntime(handle);
    return BuildTextResponse(RunCryptoInspect(ToRequestJsonView(request_json)));
  } catch (const std::exception& error) {
    return BuildFailureResponse(error.what());
  } catch (...) {
    return BuildFailureResponse(
        "tracer_core_runtime_crypto_inspect_json failed unexpectedly.");
  }
}

