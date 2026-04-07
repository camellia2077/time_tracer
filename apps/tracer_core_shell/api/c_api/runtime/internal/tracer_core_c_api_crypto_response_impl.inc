[[nodiscard]] auto BuildEncryptTextOutput(
    const app_dto::TracerExchangeExportResult& result,
    const app_dto::TracerExchangeExportRequest& request)
    -> tracer_core::core::dto::TextOutput {
  if (!result.ok) {
    return {.ok = false, .content = "", .error_message = result.error_message};
  }
  return {
      .ok = true,
      .content = "Exported complete tracer exchange package: " +
                 fs::absolute(request.input_text_root_path).string() + " -> " +
                 result.resolved_output_tracer_path.string() +
                 "\n"
                 "Included payload TXT files: " +
                 std::to_string(result.payload_file_count) +
                 "\n"
                 "Included converter TOML files: " +
                 std::to_string(result.converter_file_count) +
                 "\n"
                 "Manifest included: " +
                 std::string(result.manifest_included ? "yes" : "no"),
      .error_message = "",
  };
}

[[nodiscard]] auto BuildImportTextOutput(
    const app_dto::TracerExchangeImportResult& result,
    const app_dto::TracerExchangeImportRequest& request)
    -> tracer_core::core::dto::TextOutput {
  if (!result.ok) {
    return {.ok = false, .content = "", .error_message = result.error_message};
  }

  std::string content = "Imported complete tracer exchange package: " +
                        fs::absolute(request.input_tracer_path).string() +
                        "\n"
                        "Updated active text root: " +
                        request.active_text_root_path.string() +
                        "\n"
                        "Applied converter config: " +
                        std::string(result.config_applied ? "yes" : "no") +
                        "\n"
                        "Replaced months: " +
                        std::to_string(result.replaced_month_count) +
                        "\n"
                        "Preserved local months: " +
                        std::to_string(result.preserved_month_count) +
                        "\n"
                        "Rebuilt months: " +
                        std::to_string(result.rebuilt_month_count) +
                        "\n"
                        "Database rebuilt: " +
                        std::string(result.database_rebuilt ? "yes" : "no");
  if (result.backup_retained_root.has_value()) {
    content +=
        "\nRetained backup root: " + result.backup_retained_root->string();
    if (!result.backup_cleanup_error.empty()) {
      content += " | " + result.backup_cleanup_error;
    }
  } else if (!result.backup_cleanup_error.empty()) {
    content += "\nCleanup warning: " + result.backup_cleanup_error;
  }

  return {.ok = true, .content = std::move(content), .error_message = ""};
}

[[nodiscard]] auto BuildInspectTextOutput(
    const app_dto::TracerExchangeInspectResult& result)
    -> tracer_core::core::dto::TextOutput {
  if (!result.ok) {
    return {.ok = false, .content = "", .error_message = result.error_message};
  }
  return {
      .ok = true,
      .content =
          tracer_core::shell::tracer_exchange::BuildInspectContent(result),
      .error_message = "",
  };
}

[[nodiscard]] auto BuildUnpackTextOutput(
    const app_dto::TracerExchangeUnpackResult& result,
    const app_dto::TracerExchangeUnpackRequest& request)
    -> tracer_core::core::dto::TextOutput {
  if (!result.ok) {
    return {.ok = false, .content = "", .error_message = result.error_message};
  }
  return {
      .ok = true,
      .content = "Unpacked tracer exchange package: " +
                 fs::absolute(request.input_tracer_path).string() + " -> " +
                 result.resolved_output_root_path.string() + "\n" +
                 "Included payload TXT files: " +
                 std::to_string(result.payload_file_count) + "\n" +
                 "Included converter TOML files: " +
                 std::to_string(result.converter_file_count) + "\n" +
                 "Manifest included: " +
                 std::string(result.manifest_included ? "yes" : "no"),
      .error_message = "",
  };
}
