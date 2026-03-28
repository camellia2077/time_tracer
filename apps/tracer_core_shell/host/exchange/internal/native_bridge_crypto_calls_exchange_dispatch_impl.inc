constexpr std::string_view kProducerPlatform = "android";
constexpr std::string_view kProducerApp = "time_tracer_android";

auto ResolveActiveTextRootPath(
    const fs::path& active_converter_main_config_path) -> fs::path {
  const fs::path main_config = fs::absolute(active_converter_main_config_path);
  const fs::path runtime_root =
      main_config.parent_path().parent_path().parent_path();
  return runtime_root / "input";
}

auto WriteAllBytesToFd(int fd, std::span<const std::uint8_t> bytes,
                       std::string& error_message) -> bool {
  std::size_t offset = 0;
  while (offset < bytes.size()) {
#if defined(_WIN32)
    const int chunk_size = static_cast<int>(std::min<std::size_t>(
        bytes.size() - offset, static_cast<std::size_t>(INT_MAX)));
    const int written = _write(fd, bytes.data() + offset, chunk_size);
#else
    const ssize_t written = ::write(fd, bytes.data() + offset,
                                    bytes.size() - offset);
#endif
    if (written <= 0) {
      error_message = "Failed to write encrypted output fd.";
      return false;
    }
    offset += static_cast<std::size_t>(written);
  }
  return true;
}

auto CloseFd(int fd, std::string& error_message) -> bool {
#if defined(_WIN32)
  const int close_result = _close(fd);
#else
  const int close_result = ::close(fd);
#endif
  if (close_result != 0) {
    error_message = "Failed to close encrypted output fd.";
    return false;
  }
  return true;
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
      .logical_source_root_name = "data",
      .output_display_name = fs::path(args.output_path).filename().string(),
      .producer_platform = std::string(kProducerPlatform),
      .producer_app = std::string(kProducerApp),
      .security_level = args.security_level,
      .progress_observer = BuildTracerExchangeProgressObserver(env),
  };
}

auto BuildExportTracerExchangeFromPayloadRequest(
    JNIEnv* env, const ExportTracerExchangeFromPayloadArgs& args,
    const fs::path& converter_config_toml_path)
    -> app_dto::TracerExchangeExportRequest {
  return {
      .input_text_payloads = args.payload_items,
      .encrypted_output_writer =
          [output_fd = args.output_fd,
           consumed = false](std::span<const std::uint8_t> ciphertext_bytes,
                             std::string& error_message) mutable -> bool {
            if (consumed || output_fd < 0) {
              error_message = "Encrypted output fd has already been consumed.";
              return false;
            }
            consumed = true;
            const int fd = output_fd;
            output_fd = -1;
            const bool write_ok =
                WriteAllBytesToFd(fd, ciphertext_bytes, error_message);
            const bool close_ok = CloseFd(fd, error_message);
            return write_ok && close_ok;
          },
      .active_converter_main_config_path = converter_config_toml_path,
      .date_check_mode = args.date_check_mode,
      .passphrase = args.passphrase,
      .logical_source_root_name = args.logical_source_root_name,
      .output_display_name = args.output_display_name,
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

auto DispatchExportTracerExchangeFromPayload(
    JNIEnv* env, const ExportTracerExchangeFromPayloadArgs& args)
    -> app_dto::TracerExchangeExportResult {
  return WithLockedRuntime([&](ITracerCoreRuntime& runtime,
                               const fs::path& converter_config_toml_path) {
    return runtime.tracer_exchange().RunTracerExchangeExport(
        BuildExportTracerExchangeFromPayloadRequest(env, args,
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
