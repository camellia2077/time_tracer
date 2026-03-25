constexpr std::string_view kProducerPlatform = "android";
constexpr std::string_view kProducerApp = "time_tracer_android";

auto ResolveActiveTextRootPath(
    const fs::path& active_converter_main_config_path) -> fs::path {
  const fs::path main_config = fs::absolute(active_converter_main_config_path);
  const fs::path runtime_root =
      main_config.parent_path().parent_path().parent_path();
  return runtime_root / "input" / "full";
}

template <typename Callback>
auto WithLockedRuntime(Callback callback) {
  std::scoped_lock lock(g_runtime_mutex);
  if (g_runtime.core_runtime == nullptr ||
      g_runtime.core_runtime->runtime.runtime_api == nullptr) {
    throw std::runtime_error("nativeInit must be called first.");
  }

  ITracerCoreRuntime& runtime = *g_runtime.core_runtime->runtime.runtime_api;
  return callback(runtime, g_runtime.core_runtime->converter_config_toml_path);
}

auto BuildExportTracerExchangeRequest(
    JNIEnv* env, const ExportTracerExchangeArgs& args,
    const fs::path& converter_config_toml_path)
    -> app_dto::TracerExchangeExportRequest {
  return {
      .input_text_root_path = fs::absolute(args.input_path),
      .requested_output_path = fs::absolute(args.output_path),
      .active_converter_main_config_path = converter_config_toml_path,
      .date_check_mode = args.date_check_mode,
      .passphrase = args.passphrase,
      .producer_platform = std::string(kProducerPlatform),
      .producer_app = std::string(kProducerApp),
      .security_level = args.security_level,
      .progress_observer = BuildTracerExchangeProgressObserver(env),
  };
}

auto BuildImportTracerExchangeRequest(
    JNIEnv* env, const ImportTracerExchangeArgs& args,
    const fs::path& converter_config_toml_path)
    -> app_dto::TracerExchangeImportRequest {
  return {
      .input_tracer_path = fs::absolute(args.input_path),
      .active_text_root_path =
          ResolveActiveTextRootPath(converter_config_toml_path),
      .active_converter_main_config_path = converter_config_toml_path,
      .runtime_work_root = fs::absolute(args.work_root),
      .passphrase = args.passphrase,
      .progress_observer = BuildTracerExchangeProgressObserver(env),
  };
}

auto BuildInspectTracerExchangeRequest(JNIEnv* env,
                                       const InspectTracerExchangeArgs& args)
    -> app_dto::TracerExchangeInspectRequest {
  return {
      .input_tracer_path = fs::absolute(args.input_path),
      .passphrase = args.passphrase,
      .progress_observer = BuildTracerExchangeProgressObserver(env),
  };
}

auto DispatchExportTracerExchange(JNIEnv* env,
                                  const ExportTracerExchangeArgs& args)
    -> app_dto::TracerExchangeExportResult {
  return WithLockedRuntime([&](ITracerCoreRuntime& runtime,
                               const fs::path& converter_config_toml_path) {
    return runtime.tracer_exchange().RunTracerExchangeExport(
        BuildExportTracerExchangeRequest(env, args,
                                         converter_config_toml_path));
  });
}

auto DispatchImportTracerExchange(JNIEnv* env,
                                  const ImportTracerExchangeArgs& args)
    -> app_dto::TracerExchangeImportResult {
  return WithLockedRuntime([&](ITracerCoreRuntime& runtime,
                               const fs::path& converter_config_toml_path) {
    return runtime.tracer_exchange().RunTracerExchangeImport(
        BuildImportTracerExchangeRequest(env, args,
                                         converter_config_toml_path));
  });
}

auto DispatchInspectTracerExchange(JNIEnv* env,
                                   const InspectTracerExchangeArgs& args)
    -> app_dto::TracerExchangeInspectResult {
  return WithLockedRuntime(
      [&](ITracerCoreRuntime& runtime, const fs::path& /*unused*/) {
        return runtime.tracer_exchange().RunTracerExchangeInspect(
            BuildInspectTracerExchangeRequest(env, args));
      });
}
