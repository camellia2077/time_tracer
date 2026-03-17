// api/c_api/tracer_core_c_api_internal.hpp
#ifndef API_CORE_C_TRACER_CORE_C_API_INTERNAL_H_
#define API_CORE_C_TRACER_CORE_C_API_INTERNAL_H_

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "api/c_api/tracer_core_c_api.h"
#include "host/android_runtime_factory.hpp"
#include "domain/types/date_check_mode.hpp"

enum class IngestMode;
enum class ReportFormat;

namespace tracer::core::application::use_cases {

class ITracerCoreApi;

}  // namespace tracer::core::application::use_cases

namespace tracer_core::core::dto {

enum class DataQueryAction;
enum class DataQueryOutputMode;
enum class ReportExportType;
enum class ReportQueryType;

struct OperationAck;
struct TextOutput;
struct TreeQueryResponse;

}  // namespace tracer_core::core::dto

struct TtCoreRuntimeHandle {
  infrastructure::bootstrap::AndroidRuntime runtime;
};

namespace tracer_core::core::c_api::internal {

struct CryptoProgressCallbackRegistration {
  TtCoreCryptoProgressCallback callback = nullptr;
  void* user_data = nullptr;
};

struct CliGlobalDefaultsContext {
  std::optional<std::filesystem::path> db_path;
  std::optional<std::filesystem::path> output_root;
  std::optional<std::string> default_format;
};

struct CliCommandDefaultsContext {
  std::optional<std::string> export_format;
  std::optional<std::string> query_format;
  std::optional<DateCheckMode> convert_date_check_mode;
  std::optional<bool> convert_save_processed_output;
  std::optional<bool> convert_validate_logic;
  std::optional<bool> convert_validate_structure;
  std::optional<DateCheckMode> ingest_date_check_mode;
  std::optional<bool> ingest_save_processed_output;
  std::optional<DateCheckMode> validate_logic_date_check_mode;
};

struct CliConfigContext {
  std::filesystem::path exe_dir_path;
  std::optional<std::filesystem::path> export_path;
  std::filesystem::path converter_config_toml_path;
  bool default_save_processed_output = false;
  DateCheckMode default_date_check_mode = DateCheckMode::kNone;
  CliGlobalDefaultsContext defaults;
  CliCommandDefaultsContext command_defaults;
};

struct ResolvedCliContext {
  CliConfigContext cli_config;
  std::filesystem::path output_root;
  std::filesystem::path db_path;
  std::filesystem::path export_root;
  std::filesystem::path runtime_output_root;
};

extern thread_local std::string g_last_error;
extern thread_local std::string g_last_response;

void ClearLastError();
void SetLastError(const char* message);

[[nodiscard]] auto BuildFailureResponse(std::string message) -> const char*;
[[nodiscard]] auto BuildFailureResponse(std::string message,
                                        std::string error_code,
                                        std::string error_category,
                                        std::vector<std::string> hints) -> const
    char*;
[[nodiscard]] auto BuildOperationResponse(
    const tracer_core::core::dto::OperationAck& output) -> const char*;
[[nodiscard]] auto BuildTextResponse(
    const tracer_core::core::dto::TextOutput& output) -> const char*;
[[nodiscard]] auto BuildTreeResponse(
    const tracer_core::core::dto::TreeQueryResponse& response) -> const char*;
[[nodiscard]] auto BuildCapabilitiesResponseJson() -> const char*;
[[nodiscard]] auto ResolveCliContext(const char* executable_path,
                                     const char* db_override,
                                     const char* output_override,
                                     const char* command_name)
    -> ResolvedCliContext;

[[nodiscard]] auto RequireRuntime(TtCoreRuntimeHandle* handle)
    -> tracer::core::application::use_cases::ITracerCoreApi&;

[[nodiscard]] auto ToRequestJsonView(const char* request_json)
    -> std::string_view;

[[nodiscard]] auto ParseDateCheckMode(const std::string& value)
    -> DateCheckMode;
[[nodiscard]] auto ParseIngestMode(const std::string& value) -> IngestMode;
[[nodiscard]] auto ParseQueryAction(const std::string& value)
    -> tracer_core::core::dto::DataQueryAction;
[[nodiscard]] auto ParseDataQueryOutputMode(const std::string& value)
    -> tracer_core::core::dto::DataQueryOutputMode;
[[nodiscard]] auto ParseExportType(const std::string& value)
    -> tracer_core::core::dto::ReportExportType;
[[nodiscard]] auto ParseReportType(const std::string& value)
    -> tracer_core::core::dto::ReportQueryType;
[[nodiscard]] auto ParseReportFormat(const std::string& value) -> ReportFormat;

void SetCryptoProgressCallbackRegistration(TtCoreCryptoProgressCallback callback,
                                           void* user_data);
[[nodiscard]] auto GetCryptoProgressCallbackRegistration()
    -> CryptoProgressCallbackRegistration;

}  // namespace tracer_core::core::c_api::internal

#endif  // API_CORE_C_TRACER_CORE_C_API_INTERNAL_H_

