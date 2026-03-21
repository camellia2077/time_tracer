void ClearLastError() {
  g_last_error.clear();
}

void SetLastError(const char* message) {
  try {
    g_last_error.assign(message != nullptr ? message : "Unknown error.");
  } catch (...) {
    g_last_error = "Failed to set last error.";
  }
}

[[nodiscard]] auto BuildFailureResponse(std::string message) -> const char* {
  return BuildFailureResponse(
      std::move(message), std::string(kRuntimeGenericErrorCode),
      std::string(kRuntimeErrorCategory), std::vector<std::string>{});
}

[[nodiscard]] auto BuildFailureResponse(std::string message,
                                        std::string error_code,
                                        std::string error_category,
                                        std::vector<std::string> hints) -> const
    char* {
  if (message.empty()) {
    message = "Unknown error.";
  }
  SetLastError(message.c_str());
  g_last_response =
      tt_transport::EncodeIngestResponse(tt_transport::IngestResponsePayload{
          .ok = false,
          .error_message = message,
          .error_contract = BuildErrorContract(
              false, message,
              ErrorContractBuildOptions{
                  .error_code = error_code,
                  .error_category = error_category,
                  .hints = std::move(hints),
              }),
      });
  return g_last_response.c_str();
}

[[nodiscard]] auto BuildOperationResponse(
    const tracer_core::core::dto::OperationAck& output) -> const char* {
  return BuildAckResponse(output.ok, output.error_message);
}

[[nodiscard]] auto BuildTextResponse(
    const tracer_core::core::dto::TextOutput& output) -> const char* {
  g_last_response =
      tt_transport::EncodeQueryResponse(tt_transport::QueryResponsePayload{
          .ok = output.ok,
          .error_message = output.error_message,
          .content = output.content,
          .error_contract = BuildErrorContract(output.ok, output.error_message),
      });
  return g_last_response.c_str();
}

[[nodiscard]] auto BuildTreeResponse(
    const tracer_core::core::dto::TreeQueryResponse& response) -> const char* {
  tt_transport::TreeResponsePayload payload{};
  payload.ok = response.ok;
  payload.found = response.found;
  payload.error_message = response.error_message;
  payload.error_contract =
      BuildErrorContract(response.ok, response.error_message);
  payload.roots = response.roots;
  payload.nodes.reserve(response.tree.nodes.size());
  for (const auto& node : response.tree.nodes) {
    payload.nodes.push_back(ConvertTreeNode(node));
  }
  g_last_response = tt_transport::EncodeTreeResponse(payload);
  return g_last_response.c_str();
}

[[nodiscard]] auto BuildCapabilitiesResponseJson() -> const char* {
  tt_transport::CapabilitiesResponsePayload payload{};
  payload.abi.name = "tracer_core_c";
  payload.abi.version = 1;
  payload.features.build_info_json = true;
  payload.features.command_contract_json = true;
  payload.features.runtime_log_callback = true;
  payload.features.runtime_diagnostics_callback = true;
  payload.features.runtime_crypto_progress_callback = true;
  payload.features.runtime_ingest_json = true;
  payload.features.runtime_convert_json = true;
  payload.features.runtime_import_json = true;
  payload.features.runtime_validate_structure_json = true;
  payload.features.runtime_validate_logic_json = true;
  payload.features.runtime_query_json = true;
  payload.features.runtime_report_json = true;
  payload.features.runtime_report_batch_json = true;
  payload.features.runtime_export_json = true;
  payload.features.runtime_tree_json = true;
  payload.features.processed_json_io = TT_ENABLE_PROCESSED_JSON_IO != 0;
  payload.features.report_markdown = true;
  payload.features.report_latex = TT_REPORT_ENABLE_LATEX != 0;
  payload.features.report_typst = TT_REPORT_ENABLE_TYPST != 0;
  g_last_response = tt_transport::EncodeCapabilitiesResponse(payload);
  return g_last_response.c_str();
}

[[nodiscard]] auto RequireRuntime(TtCoreRuntimeHandle* handle)
    -> ITracerCoreRuntime& {
  if (handle == nullptr || handle->runtime.runtime_api == nullptr) {
    throw std::invalid_argument("runtime handle is null.");
  }
  return *handle->runtime.runtime_api;
}

[[nodiscard]] auto ToRequestJsonView(const char* request_json)
    -> std::string_view {
  return request_json != nullptr ? std::string_view(request_json)
                                 : std::string_view{};
}

[[nodiscard]] auto ParseDateCheckMode(const std::string& value)
    -> DateCheckMode {
  return tracer_core::shell::c_api_bridge::ParseDateCheckMode(value);
}

[[nodiscard]] auto ParseIngestMode(const std::string& value) -> IngestMode {
  return tracer_core::shell::c_api_bridge::ParseIngestMode(value);
}

[[nodiscard]] auto ParseQueryAction(const std::string& value)
    -> tracer_core::core::dto::DataQueryAction {
  return tracer_core::shell::c_api_bridge::ParseQueryAction(value);
}

[[nodiscard]] auto ParseDataQueryOutputMode(const std::string& value)
    -> tracer_core::core::dto::DataQueryOutputMode {
  return tracer_core::shell::c_api_bridge::ParseDataQueryOutputMode(value);
}

[[nodiscard]] auto ParseExportType(const std::string& value)
    -> tracer_core::core::dto::ReportExportType {
  return tracer_core::shell::c_api_bridge::ParseExportType(value);
}

[[nodiscard]] auto ParseReportType(const std::string& value)
    -> tracer_core::core::dto::ReportQueryType {
  return tracer_core::shell::c_api_bridge::ParseReportType(value);
}

[[nodiscard]] auto ParseReportFormat(const std::string& value) -> ReportFormat {
  return tracer_core::shell::c_api_bridge::ParseReportFormat(value);
}
