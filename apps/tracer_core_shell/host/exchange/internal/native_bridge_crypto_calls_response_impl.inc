auto BuildExportContentJson(const app_dto::TracerExchangeExportResult& result)
    -> std::string {
  return json{{"output_path", result.resolved_output_tracer_path.string()},
              {"source_root_name", result.source_root_name},
              {"payload_file_count", result.payload_file_count},
              {"converter_file_count", result.converter_file_count},
              {"manifest_included", result.manifest_included}}
      .dump();
}

auto BuildImportContentJson(const app_dto::TracerExchangeImportResult& result)
    -> std::string {
  return json{
      {"source_root_name", result.source_root_name},
      {"payload_file_count", result.payload_file_count},
      {"replaced_month_count", result.replaced_month_count},
      {"preserved_month_count", result.preserved_month_count},
      {"rebuilt_month_count", result.rebuilt_month_count},
      {"text_root_updated", result.text_root_updated},
      {"config_applied", result.config_applied},
      {"database_rebuilt", result.database_rebuilt},
      {"retained_failure_root", result.retained_failure_root.has_value()
                                    ? result.retained_failure_root->string()
                                    : std::string()},
      {"backup_retained_root", result.backup_retained_root.has_value()
                                   ? result.backup_retained_root->string()
                                   : std::string()},
      {"backup_cleanup_error", result.backup_cleanup_error},
  }
      .dump();
}

auto BuildInspectContentJson(const app_dto::TracerExchangeInspectResult& result)
    -> std::string {
  return json{
      {"input_path", result.input_tracer_path.string()},
      {"package_type", result.package_type},
      {"package_version", result.package_version},
      {"source_root_name", result.source_root_name},
      {"payload_file_count", result.payload_file_count},
      {"producer_platform", result.producer_platform},
      {"producer_app", result.producer_app},
      {"created_at_utc", result.created_at_utc},
      {"rendered_text",
       tracer_core::shell::tracer_exchange::BuildInspectContent(result)},
  }
      .dump();
}

auto BuildTracerExchangeExportResponse(
    const app_dto::TracerExchangeExportResult& result) -> std::string {
  if (!result.ok) {
    return BuildResponseJson(false,
                             result.error_message.empty()
                                 ? "complete exchange package export failed."
                                 : result.error_message,
                             std::string{});
  }
  return BuildResponseJson(true, std::string{}, BuildExportContentJson(result));
}

auto BuildTracerExchangeImportResponse(
    const app_dto::TracerExchangeImportResult& result) -> std::string {
  if (!result.ok) {
    return BuildResponseJson(false,
                             result.error_message.empty()
                                 ? "import tracer exchange failed."
                                 : result.error_message,
                             BuildImportContentJson(result));
  }
  return BuildResponseJson(true, std::string{}, BuildImportContentJson(result));
}

auto BuildTracerExchangeInspectResponse(
    const app_dto::TracerExchangeInspectResult& result) -> std::string {
  if (!result.ok) {
    return BuildResponseJson(false,
                             result.error_message.empty()
                                 ? "inspect tracer exchange failed."
                                 : result.error_message,
                             std::string{});
  }
  return BuildResponseJson(true, std::string{},
                           BuildInspectContentJson(result));
}
