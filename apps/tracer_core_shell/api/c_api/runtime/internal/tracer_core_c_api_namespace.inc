namespace {

namespace fs = std::filesystem;
using nlohmann::json;

constexpr std::string_view kCoreAbiName = "tracer_core_c";
constexpr int kCoreAbiVersion = 1;
constexpr std::string_view kCoreLibraryName = "tracer_core.dll";
constexpr std::string_view kReportsSharedLibraryName = "reports_shared.dll";
constexpr std::string_view kSqliteLibraryName = "libsqlite3-0.dll";
constexpr std::string_view kTomlLibraryName = "libtomlplusplus-3.dll";
constexpr std::string_view kLibgccRuntimeName = "libgcc_s_seh-1.dll";
constexpr std::string_view kLibstdcppRuntimeName = "libstdc++-6.dll";
constexpr std::string_view kLibwinpthreadRuntimeName = "libwinpthread-1.dll";

#ifndef TT_RUNTIME_REQUIRE_SQLITE_DLL
#define TT_RUNTIME_REQUIRE_SQLITE_DLL 1
#endif

#ifndef TT_RUNTIME_REQUIRE_TOML_DLL
#define TT_RUNTIME_REQUIRE_TOML_DLL 1
#endif

#ifndef TT_RUNTIME_REQUIRE_MINGW_DLLS
#define TT_RUNTIME_REQUIRE_MINGW_DLLS 1
#endif

struct LogCallbackRegistration {
  TtCoreLogCallback callback = nullptr;
  void* user_data = nullptr;
};

struct DiagnosticsCallbackRegistration {
  TtCoreDiagnosticsCallback callback = nullptr;
  void* user_data = nullptr;
};

std::mutex g_callback_mutex;
LogCallbackRegistration g_log_callback_registration{};
DiagnosticsCallbackRegistration g_diagnostics_callback_registration{};

[[nodiscard]] auto IsNonEmptyCString(const char* value) -> bool {
  return value != nullptr && value[0] != '\0';
}

auto GetLogCallbackRegistration() -> LogCallbackRegistration {
  std::scoped_lock lock(g_callback_mutex);
  return g_log_callback_registration;
}

auto GetDiagnosticsCallbackRegistration() -> DiagnosticsCallbackRegistration {
  std::scoped_lock lock(g_callback_mutex);
  return g_diagnostics_callback_registration;
}

auto ToAbiLogSeverity(
    tracer_core::application::runtime_bridge::LogSeverity severity)
    -> TtCoreLogSeverity {
  switch (severity) {
    case tracer_core::application::runtime_bridge::LogSeverity::kInfo:
      return TT_CORE_LOG_INFO;
    case tracer_core::application::runtime_bridge::LogSeverity::kWarn:
      return TT_CORE_LOG_WARN;
    case tracer_core::application::runtime_bridge::LogSeverity::kError:
      return TT_CORE_LOG_ERROR;
  }
  return TT_CORE_LOG_INFO;
}

auto ToAbiDiagnosticSeverity(
    tracer_core::domain::ports::DiagnosticSeverity severity)
    -> TtCoreDiagnosticSeverity {
  switch (severity) {
    case tracer_core::domain::ports::DiagnosticSeverity::kInfo:
      return TT_CORE_DIAGNOSTIC_INFO;
    case tracer_core::domain::ports::DiagnosticSeverity::kWarn:
      return TT_CORE_DIAGNOSTIC_WARN;
    case tracer_core::domain::ports::DiagnosticSeverity::kError:
      return TT_CORE_DIAGNOSTIC_ERROR;
  }
  return TT_CORE_DIAGNOSTIC_INFO;
}

class AbiLogCallbackLogger final
    : public tracer_core::application::runtime_bridge::ILogger {
 public:
  auto Log(tracer_core::application::runtime_bridge::LogSeverity severity,
           std::string_view message) -> void override {
    const LogCallbackRegistration registration = GetLogCallbackRegistration();
    if (registration.callback == nullptr) {
      return;
    }
    std::string utf8_message(message);
    try {
      registration.callback(ToAbiLogSeverity(severity), utf8_message.c_str(),
                            registration.user_data);
    } catch (...) {
      // Host callback failures must not interrupt core execution.
    }
  }
};

class AbiDiagnosticsCallbackSink final
    : public tracer_core::domain::ports::IDiagnosticsSink {
 public:
  auto Emit(tracer_core::domain::ports::DiagnosticSeverity severity,
            std::string_view message) -> void override {
    const DiagnosticsCallbackRegistration registration =
        GetDiagnosticsCallbackRegistration();
    if (registration.callback == nullptr) {
      return;
    }
    std::string utf8_message(message);
    try {
      registration.callback(ToAbiDiagnosticSeverity(severity),
                            utf8_message.c_str(), registration.user_data);
    } catch (...) {
      // Host callback failures must not interrupt core execution.
    }
  }
};

auto CreateAbiLoggerAdapter()
    -> std::shared_ptr<tracer_core::application::runtime_bridge::ILogger> {
  return std::make_shared<AbiLogCallbackLogger>();
}

auto CreateAbiDiagnosticsAdapter()
    -> std::shared_ptr<tracer_core::domain::ports::IDiagnosticsSink> {
  return std::make_shared<AbiDiagnosticsCallbackSink>();
}

[[nodiscard]] auto ToDateCheckModeString(DateCheckMode mode)
    -> std::string_view {
  switch (mode) {
    case DateCheckMode::kNone:
      return "none";
    case DateCheckMode::kContinuity:
      return "continuity";
    case DateCheckMode::kFull:
      return "full";
  }
  return "none";
}

[[nodiscard]] auto ToDateCheckModeJson(const std::optional<DateCheckMode>& mode)
    -> json {
  if (!mode.has_value()) {
    return nullptr;
  }
  return std::string(ToDateCheckModeString(*mode));
}

[[nodiscard]] auto ToStringJson(const std::optional<std::string>& value)
    -> json {
  if (!value.has_value()) {
    return nullptr;
  }
  return *value;
}

[[nodiscard]] auto BuildCliConfigJson(
    const tracer_core::core::c_api::internal::CliConfigContext& cli_config)
    -> json {
  return json{
      {"default_save_processed_output",
       cli_config.default_save_processed_output},
      {"default_date_check_mode",
       std::string(ToDateCheckModeString(cli_config.default_date_check_mode))},
      {"defaults",
       {{"default_format", ToStringJson(cli_config.defaults.default_format)}}},
      {"command_defaults",
       {{"export_format",
         ToStringJson(cli_config.command_defaults.export_format)},
        {"query_format",
         ToStringJson(cli_config.command_defaults.query_format)},
        {"convert_date_check_mode",
         ToDateCheckModeJson(
             cli_config.command_defaults.convert_date_check_mode)},
        {"convert_save_processed_output",
         cli_config.command_defaults.convert_save_processed_output.has_value()
             ? json(*cli_config.command_defaults.convert_save_processed_output)
             : json(nullptr)},
        {"convert_validate_logic",
         cli_config.command_defaults.convert_validate_logic.has_value()
             ? json(*cli_config.command_defaults.convert_validate_logic)
             : json(nullptr)},
        {"convert_validate_structure",
         cli_config.command_defaults.convert_validate_structure.has_value()
             ? json(*cli_config.command_defaults.convert_validate_structure)
             : json(nullptr)},
        {"ingest_date_check_mode",
         ToDateCheckModeJson(
             cli_config.command_defaults.ingest_date_check_mode)},
        {"ingest_save_processed_output",
         cli_config.command_defaults.ingest_save_processed_output.has_value()
             ? json(*cli_config.command_defaults.ingest_save_processed_output)
             : json(nullptr)},
        {"validate_logic_date_check_mode",
         ToDateCheckModeJson(
             cli_config.command_defaults.validate_logic_date_check_mode)}}}};
}

[[nodiscard]] auto BuildFailureJsonResponse(
    std::string_view error_message,
    const std::vector<std::string>& messages = {},
    std::string_view error_code = "runtime.generic_error",
    std::string_view error_category = "runtime",
    std::vector<std::string> hints = {}) -> const char* {
  const std::string kNormalizedError = error_message.empty()
                                           ? std::string("Unknown error.")
                                           : std::string(error_message);
  SetLastError(kNormalizedError.c_str());

  if (hints.empty()) {
    hints.emplace_back("Inspect `error_message` for detailed failure reason.");
  }
  json payload = {{"ok", false},
                  {"error_message", kNormalizedError},
                  {"error_code", std::string(error_code)},
                  {"error_category", std::string(error_category)},
                  {"hints", std::move(hints)}};
  if (!messages.empty()) {
    payload["messages"] = messages;
  }
  tracer_core::core::c_api::internal::g_last_response = payload.dump();
  return tracer_core::core::c_api::internal::g_last_response.c_str();
}

[[nodiscard]] auto BuildSuccessJsonResponse(const json& payload) -> const
    char* {
  json normalized = payload;
  normalized["error_code"] = "";
  normalized["error_category"] = "";
  normalized["hints"] = json::array();
  tracer_core::core::c_api::internal::g_last_response = normalized.dump();
  return tracer_core::core::c_api::internal::g_last_response.c_str();
}

}  // namespace
