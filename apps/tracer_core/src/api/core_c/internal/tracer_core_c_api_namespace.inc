namespace {

namespace fs = std::filesystem;
using nlohmann::json;

constexpr std::string_view kDatabaseFilename = "time_data.sqlite3";
constexpr std::string_view kCoreAbiName = "tracer_core_c";
constexpr int kCoreAbiVersion = 1;
constexpr std::string_view kCoreLibraryName = "tracer_core.dll";
constexpr std::string_view kReportsSharedLibraryName = "libreports_shared.dll";
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

struct ResolvedCliContext {
  AppConfig app_config;
  fs::path output_root;
  fs::path db_path;
  fs::path export_root;
  fs::path runtime_output_root;
};

struct LogCallbackRegistration {
  TtCoreLogCallback callback = nullptr;
  void* user_data = nullptr;
};

struct DiagnosticsCallbackRegistration {
  TtCoreDiagnosticsCallback callback = nullptr;
  void* user_data = nullptr;
};

struct AppConfigCacheEntry {
  fs::path normalized_executable_path;
  AppConfig app_config;
};

std::mutex g_callback_mutex;
LogCallbackRegistration g_log_callback_registration{};
DiagnosticsCallbackRegistration g_diagnostics_callback_registration{};
std::mutex g_app_config_cache_mutex;
std::optional<AppConfigCacheEntry> g_app_config_cache{};

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

auto ToAbiLogSeverity(tracer_core::application::ports::LogSeverity severity)
    -> TtCoreLogSeverity {
  switch (severity) {
    case tracer_core::application::ports::LogSeverity::kInfo:
      return TT_CORE_LOG_INFO;
    case tracer_core::application::ports::LogSeverity::kWarn:
      return TT_CORE_LOG_WARN;
    case tracer_core::application::ports::LogSeverity::kError:
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
    : public tracer_core::application::ports::ILogger {
 public:
  auto Log(tracer_core::application::ports::LogSeverity severity,
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
    -> std::shared_ptr<tracer_core::application::ports::ILogger> {
  return std::make_shared<AbiLogCallbackLogger>();
}

auto CreateAbiDiagnosticsAdapter()
    -> std::shared_ptr<tracer_core::domain::ports::IDiagnosticsSink> {
  return std::make_shared<AbiDiagnosticsCallbackSink>();
}

[[nodiscard]] auto NormalizeExecutablePath(const fs::path& executable_path)
    -> fs::path {
  std::error_code error;
  const fs::path canonical = fs::weakly_canonical(executable_path, error);
  if (!error) {
    return canonical;
  }
  return fs::absolute(executable_path);
}

[[nodiscard]] auto LoadAppConfigCached(const fs::path& executable_path)
    -> AppConfig {
  const fs::path normalized_path = NormalizeExecutablePath(executable_path);
  {
    std::scoped_lock lock(g_app_config_cache_mutex);
    if (g_app_config_cache.has_value() &&
        g_app_config_cache->normalized_executable_path == normalized_path) {
      return g_app_config_cache->app_config;
    }
  }

  const std::string executable_utf8 = normalized_path.string();
  ConfigLoader config_loader(executable_utf8.c_str());
  AppConfig app_config = config_loader.LoadConfiguration();

  {
    std::scoped_lock lock(g_app_config_cache_mutex);
    g_app_config_cache = AppConfigCacheEntry{
        .normalized_executable_path = normalized_path,
        .app_config = app_config,
    };
  }
  return app_config;
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

[[nodiscard]] auto BuildCliConfigJson(const AppConfig& app_config) -> json {
  return json{
      {"default_save_processed_output",
       app_config.default_save_processed_output},
      {"default_date_check_mode",
       std::string(ToDateCheckModeString(app_config.default_date_check_mode))},
      {"defaults",
       {{"default_format", ToStringJson(app_config.defaults.default_format)}}},
      {"command_defaults",
       {{"export_format",
         ToStringJson(app_config.command_defaults.export_format)},
        {"query_format",
         ToStringJson(app_config.command_defaults.query_format)},
        {"convert_date_check_mode",
         ToDateCheckModeJson(
             app_config.command_defaults.convert_date_check_mode)},
        {"convert_save_processed_output",
         app_config.command_defaults.convert_save_processed_output.has_value()
             ? json(*app_config.command_defaults.convert_save_processed_output)
             : json(nullptr)},
        {"convert_validate_logic",
         app_config.command_defaults.convert_validate_logic.has_value()
             ? json(*app_config.command_defaults.convert_validate_logic)
             : json(nullptr)},
        {"convert_validate_structure",
         app_config.command_defaults.convert_validate_structure.has_value()
             ? json(*app_config.command_defaults.convert_validate_structure)
             : json(nullptr)},
        {"ingest_date_check_mode",
         ToDateCheckModeJson(
             app_config.command_defaults.ingest_date_check_mode)},
        {"ingest_save_processed_output",
         app_config.command_defaults.ingest_save_processed_output.has_value()
             ? json(*app_config.command_defaults.ingest_save_processed_output)
             : json(nullptr)},
        {"validate_logic_date_check_mode",
         ToDateCheckModeJson(
             app_config.command_defaults.validate_logic_date_check_mode)}}}};
}

[[nodiscard]] auto ResolveCliContext(const char* executable_path,
                                     const char* db_override,
                                     const char* output_override,
                                     const char* command_name)
    -> ResolvedCliContext {
  if (!IsNonEmptyCString(executable_path)) {
    throw std::invalid_argument("executable_path must not be empty.");
  }

  const fs::path kExecutable = fs::absolute(fs::path(executable_path));
  AppConfig app_config = LoadAppConfigCached(kExecutable);
  fs::path output_root = fs::absolute(kExecutable.parent_path() / "output");
  if (app_config.defaults.output_root.has_value()) {
    output_root = fs::absolute(*app_config.defaults.output_root);
  }

  fs::path export_root;
  if (IsNonEmptyCString(output_override)) {
    export_root = fs::absolute(fs::path(output_override));
  } else if (app_config.kExportPath.has_value()) {
    export_root = fs::absolute(*app_config.kExportPath);
  }

  fs::path db_path = output_root / "db" / kDatabaseFilename;
  if (app_config.defaults.kDbPath.has_value()) {
    db_path = fs::absolute(*app_config.defaults.kDbPath);
  }
  if (IsNonEmptyCString(db_override)) {
    db_path = fs::absolute(fs::path(db_override));
  }

  const std::string kResolvedCommand = IsNonEmptyCString(command_name)
                                           ? std::string(command_name)
                                           : std::string();
  const fs::path kRuntimeOutputRoot =
      (kResolvedCommand == "export" && !export_root.empty()) ? export_root
                                                             : output_root;

  return ResolvedCliContext{
      .app_config = std::move(app_config),
      .output_root = std::move(output_root),
      .db_path = std::move(db_path),
      .export_root = std::move(export_root),
      .runtime_output_root = kRuntimeOutputRoot,
  };
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
