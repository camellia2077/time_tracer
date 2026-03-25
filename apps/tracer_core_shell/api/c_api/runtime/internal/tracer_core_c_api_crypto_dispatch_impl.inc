constexpr std::string_view kProducerPlatform = "windows";
constexpr std::string_view kProducerApp = "time_tracer_cli";

struct CryptoEncryptExecution {
  app_dto::TracerExchangeExportRequest request;
  app_dto::TracerExchangeExportResult result;
};

struct CryptoDecryptExecution {
  app_dto::TracerExchangeImportRequest request;
  app_dto::TracerExchangeImportResult result;
};

struct CryptoInspectExecution {
  app_dto::TracerExchangeInspectRequest request;
  app_dto::TracerExchangeInspectResult result;
};

[[nodiscard]] auto ResolveActiveTextRootPath(
    const fs::path& active_converter_main_config_path) -> fs::path {
  const fs::path main_config = fs::absolute(active_converter_main_config_path);
  const fs::path runtime_root =
      main_config.parent_path().parent_path().parent_path();
  return runtime_root / "input" / "full";
}

[[nodiscard]] auto EmitCryptoProgress(
    const app_dto::TracerExchangeProgressSnapshot& snapshot)
    -> app_dto::TracerExchangeProgressControl {
  const auto registration = tracer_core::core::c_api::internal::
      GetCryptoProgressCallbackRegistration();
  if (registration.callback == nullptr) {
    return app_dto::TracerExchangeProgressControl::kContinue;
  }

  const std::string progress_json =
      tracer_core::shell::crypto_progress_bridge::BuildProgressSnapshotJson(
          snapshot);
  try {
    registration.callback(progress_json.c_str(), registration.user_data);
  } catch (...) {
    // Host callback failures must not interrupt core execution.
  }
  return app_dto::TracerExchangeProgressControl::kContinue;
}

[[nodiscard]] auto BuildProgressObserver()
    -> app_dto::TracerExchangeProgressObserver {
  return [](const app_dto::TracerExchangeProgressSnapshot& snapshot) {
    return EmitCryptoProgress(snapshot);
  };
}

[[nodiscard]] auto BuildCryptoEncryptRequest(TtCoreRuntimeHandle* handle,
                                             const CryptoEncryptArgs& args)
    -> app_dto::TracerExchangeExportRequest {
  return {
      .input_text_root_path = args.input_path,
      .requested_output_path = args.output_path,
      .active_converter_main_config_path = handle->converter_config_toml_path,
      .date_check_mode = args.date_check_mode,
      .passphrase = args.passphrase,
      .producer_platform = std::string(kProducerPlatform),
      .producer_app = std::string(kProducerApp),
      .security_level = args.security_level,
      .progress_observer = BuildProgressObserver(),
  };
}

[[nodiscard]] auto BuildCryptoDecryptRequest(TtCoreRuntimeHandle* handle,
                                             const CryptoDecryptArgs& args)
    -> app_dto::TracerExchangeImportRequest {
  return {
      .input_tracer_path = args.input_path,
      .active_text_root_path =
          ResolveActiveTextRootPath(handle->converter_config_toml_path),
      .active_converter_main_config_path = handle->converter_config_toml_path,
      .runtime_work_root = args.work_root_override.has_value()
                               ? *args.work_root_override
                               : handle->output_root,
      .passphrase = args.passphrase,
      .progress_observer = BuildProgressObserver(),
  };
}

[[nodiscard]] auto BuildCryptoInspectRequest(const CryptoInspectArgs& args)
    -> app_dto::TracerExchangeInspectRequest {
  return {
      .input_tracer_path = args.input_path,
      .passphrase = args.passphrase,
      .progress_observer = BuildProgressObserver(),
  };
}

[[nodiscard]] auto DispatchCryptoEncrypt(TtCoreRuntimeHandle* handle,
                                         const CryptoEncryptArgs& args)
    -> CryptoEncryptExecution {
  ITracerCoreRuntime& runtime = RequireRuntime(handle);
  CryptoEncryptExecution execution{
      .request = BuildCryptoEncryptRequest(handle, args),
  };
  execution.result =
      runtime.tracer_exchange().RunTracerExchangeExport(execution.request);
  return execution;
}

[[nodiscard]] auto DispatchCryptoDecrypt(TtCoreRuntimeHandle* handle,
                                         const CryptoDecryptArgs& args)
    -> CryptoDecryptExecution {
  ITracerCoreRuntime& runtime = RequireRuntime(handle);
  CryptoDecryptExecution execution{
      .request = BuildCryptoDecryptRequest(handle, args),
  };
  execution.result =
      runtime.tracer_exchange().RunTracerExchangeImport(execution.request);
  return execution;
}

[[nodiscard]] auto DispatchCryptoInspect(TtCoreRuntimeHandle* handle,
                                         const CryptoInspectArgs& args)
    -> CryptoInspectExecution {
  ITracerCoreRuntime& runtime = RequireRuntime(handle);
  CryptoInspectExecution execution{
      .request = BuildCryptoInspectRequest(args),
  };
  execution.result =
      runtime.tracer_exchange().RunTracerExchangeInspect(execution.request);
  return execution;
}
