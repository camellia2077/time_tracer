// api/c_api/tracer_core_c_api.h
// - All exported symbols must use `tracer_core_*` prefix.
// - Do not add or reintroduce legacy `tt_*` symbols.
// Canonical spec: docs/time_tracer/core/contracts/c_abi.md
#ifndef API_CORE_C_TRACER_CORE_C_API_H_
#define API_CORE_C_TRACER_CORE_C_API_H_

// C ABI declarations intentionally keep C naming/typedef style for
// cross-language compatibility and stable exported symbols.
// NOLINTBEGIN(readability-identifier-naming,modernize-use-trailing-return-type,modernize-use-using,readability-use-concise-preprocessor-directives)

#ifdef _WIN32
#ifdef TRACER_CORE_EXPORTS
#define TT_CORE_API \
  __declspec(dllexport)  // NOLINT(readability-identifier-naming)
#else
#define TT_CORE_API \
  __declspec(dllimport)  // NOLINT(readability-identifier-naming)
#endif
#else
#define TT_CORE_API  // NOLINT(readability-identifier-naming)
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum TtCoreStatus {
  TT_CORE_STATUS_OK = 0,
  TT_CORE_STATUS_ERROR = 1,
} TtCoreStatus;

typedef enum TtCoreLogSeverity {
  TT_CORE_LOG_INFO = 0,
  TT_CORE_LOG_WARN = 1,
  TT_CORE_LOG_ERROR = 2,
} TtCoreLogSeverity;

typedef enum TtCoreDiagnosticSeverity {
  TT_CORE_DIAGNOSTIC_INFO = 0,
  TT_CORE_DIAGNOSTIC_WARN = 1,
  TT_CORE_DIAGNOSTIC_ERROR = 2,
} TtCoreDiagnosticSeverity;

typedef void (*TtCoreLogCallback)(TtCoreLogSeverity severity,
                                  const char* utf8_message, void* user_data);
typedef void (*TtCoreDiagnosticsCallback)(TtCoreDiagnosticSeverity severity,
                                          const char* utf8_message,
                                          void* user_data);
typedef void (*TtCoreCryptoProgressCallback)(const char* utf8_progress_json,
                                             void* user_data);

typedef struct TtCoreRuntimeHandle TtCoreRuntimeHandle;

// Returns a static null-terminated UTF-8 version string.
TT_CORE_API const char* tracer_core_get_version(void);

// Connectivity/health probe for host-side bootstrap checks.
TT_CORE_API int tracer_core_ping(void);

// Returns compile-time/runtime capability map as UTF-8 JSON.
TT_CORE_API const char* tracer_core_get_capabilities_json(void);

// Returns build metadata payload as UTF-8 JSON.
TT_CORE_API const char* tracer_core_get_build_info_json(void);

// Returns command-level contract payload as UTF-8 JSON.
// `request_json` may be empty/null or include filter:
// { "command": "query" }
TT_CORE_API const char* tracer_core_get_command_contract_json(
    const char* request_json);

// Validates host runtime environment for CLI bootstrap.
// Returns UTF-8 JSON object:
// {
//   "ok": bool,
//   "error_message": string,
//   "error_code": string,
//   "error_category": string,
//   "hints": [string, ...],
//   "messages": [string, ...]
// }
TT_CORE_API const char* tracer_core_runtime_check_environment_json(
    const char* executable_path, int is_help_mode);

// Resolves CLI runtime context from config + overrides.
// Returns UTF-8 JSON object with:
// - ok / error_message
// - paths: db_path, runtime_output_root, converter_config_toml_path, ...
// - cli_config: default values for CLI command layer
TT_CORE_API const char* tracer_core_runtime_resolve_cli_context_json(
    const char* executable_path, const char* db_override,
    const char* output_override, const char* command_name);

// Returns the last thread-local error message. Empty string means no error.
TT_CORE_API const char* tracer_core_last_error(void);

// Registers host log callback for runtime logger events.
// Passing nullptr clears current callback.
TT_CORE_API void tracer_core_set_log_callback(TtCoreLogCallback callback,
                                              void* user_data);

// Registers host diagnostics callback for runtime diagnostics events.
// Passing nullptr clears current callback.
TT_CORE_API void tracer_core_set_diagnostics_callback(
    TtCoreDiagnosticsCallback callback, void* user_data);

// Registers host callback for file crypto progress events.
// Passing nullptr clears current callback.
TT_CORE_API void tracer_core_set_crypto_progress_callback(
    TtCoreCryptoProgressCallback callback, void* user_data);

// Creates a reusable core runtime handle.
// - `db_path` can be empty/null to use default under `output_root`.
// - `output_root` must be a valid writable directory path.
// - `converter_config_toml_path` points to converter config TOML.
// Returns nullptr on failure; call `tracer_core_last_error()` for diagnostics.
TT_CORE_API TtCoreRuntimeHandle* tracer_core_runtime_create(
    const char* db_path, const char* output_root,
    const char* converter_config_toml_path);

// Destroys a runtime handle created by `tracer_core_runtime_create`.
TT_CORE_API void tracer_core_runtime_destroy(TtCoreRuntimeHandle* handle);

// Runs ingest/query/report with JSON request payload and returns JSON response.
// Returned pointer is thread-local and remains valid until the next API call
// on the same thread.
// Standard response envelope fields:
// - ok / error_message / error_code / error_category / hints
// - content (for text responses)
TT_CORE_API const char* tracer_core_runtime_ingest_json(
    TtCoreRuntimeHandle* handle, const char* request_json);
TT_CORE_API const char* tracer_core_runtime_convert_json(
    TtCoreRuntimeHandle* handle, const char* request_json);
TT_CORE_API const char* tracer_core_runtime_import_json(
    TtCoreRuntimeHandle* handle, const char* request_json);
TT_CORE_API const char* tracer_core_runtime_validate_structure_json(
    TtCoreRuntimeHandle* handle, const char* request_json);
TT_CORE_API const char* tracer_core_runtime_validate_logic_json(
    TtCoreRuntimeHandle* handle, const char* request_json);
TT_CORE_API const char* tracer_core_runtime_query_json(
    TtCoreRuntimeHandle* handle, const char* request_json);
TT_CORE_API const char* tracer_core_runtime_report_json(
    TtCoreRuntimeHandle* handle, const char* request_json);
TT_CORE_API const char* tracer_core_runtime_report_batch_json(
    TtCoreRuntimeHandle* handle, const char* request_json);
TT_CORE_API const char* tracer_core_runtime_export_json(
    TtCoreRuntimeHandle* handle, const char* request_json);
TT_CORE_API const char* tracer_core_runtime_tree_json(
    TtCoreRuntimeHandle* handle, const char* request_json);

// Runs file-crypto operations with JSON request payload and returns JSON
// text-response payload:
// {
//   "ok": bool,
//   "error_message": string,
//   "content": string
// }
TT_CORE_API const char* tracer_core_runtime_crypto_encrypt_json(
    TtCoreRuntimeHandle* handle, const char* request_json);
TT_CORE_API const char* tracer_core_runtime_crypto_decrypt_json(
    TtCoreRuntimeHandle* handle, const char* request_json);
TT_CORE_API const char* tracer_core_runtime_crypto_inspect_json(
    TtCoreRuntimeHandle* handle, const char* request_json);

#ifdef __cplusplus
}
#endif

// NOLINTEND(readability-identifier-naming,modernize-use-trailing-return-type,modernize-use-using,readability-use-concise-preprocessor-directives)

#endif  // API_CORE_C_TRACER_CORE_C_API_H_
