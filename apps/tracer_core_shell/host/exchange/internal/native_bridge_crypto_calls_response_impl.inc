auto BuildExportContentJson(const app_dto::TracerExchangeExportResult& result)
    -> std::string {
  return json{{"output_path", result.resolved_output_tracer_path.string()},
              {"source_root_name", result.source_root_name},
              {"payload_file_count", result.payload_file_count},
              {"converter_file_count", result.converter_file_count},
              {"manifest_included", result.manifest_included}}
      .dump();
}

auto BuildTracerExchangeContentResponse(
    const app_dto::TracerExchangeContentResult& result) -> std::string {
  if (!result.ok) {
    return BuildResponseJson(
        false,
        result.error_message.empty() ? "build exchange content failed."
                                     : result.error_message,
        std::string{});
  }

  const auto encode_base64 = [](std::span<const std::uint8_t> bytes) {
    constexpr std::string_view kAlphabet =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string encoded;
    encoded.reserve(((bytes.size() + 2U) / 3U) * 4U);
    for (std::size_t index = 0; index < bytes.size(); index += 3U) {
      const std::size_t remaining = bytes.size() - index;
      const std::uint32_t value =
          (static_cast<std::uint32_t>(bytes[index]) << 16U) |
          (remaining > 1U
               ? static_cast<std::uint32_t>(bytes[index + 1U]) << 8U
               : 0U) |
          (remaining > 2U ? static_cast<std::uint32_t>(bytes[index + 2U])
                          : 0U);
      encoded.push_back(kAlphabet[(value >> 18U) & 0x3FU]);
      encoded.push_back(kAlphabet[(value >> 12U) & 0x3FU]);
      encoded.push_back(remaining > 1U ? kAlphabet[(value >> 6U) & 0x3FU]
                                      : '=');
      encoded.push_back(remaining > 2U ? kAlphabet[value & 0x3FU] : '=');
    }
    return encoded;
  };

  std::string manifest_text;
  json entries = json::array();
  for (const auto& entry : result.content.entries) {
    const std::string content(entry.data.begin(), entry.data.end());
    if (entry.kind == app_dto::TracerExchangeContentEntryKind::kManifest) {
      manifest_text = content;
      continue;
    }
    if (entry.text) {
      entries.push_back(json{{"relative_path", entry.relative_path},
                             {"content", content}});
    } else {
      entries.push_back(json{{"relative_path", entry.relative_path},
                             {"content_base64", encode_base64(entry.data)}});
    }
  }
  return BuildResponseJson(
      true, std::string{},
      json{{"manifest_text", manifest_text}, {"entries", entries}}.dump());
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
