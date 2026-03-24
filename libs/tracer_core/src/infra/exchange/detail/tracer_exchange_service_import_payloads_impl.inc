using exchange_pkg::DecodePackageBytes;

auto CollectImportedPayloadFiles(const fs::path& extracted_root)
    -> std::vector<ImportedPayloadFile> {
  const fs::path payload_root =
      extracted_root / fs::path(exchange_pkg::kPayloadRoot);
  if (!fs::exists(payload_root) || !fs::is_directory(payload_root)) {
    throw std::runtime_error(
        "Decoded tracer exchange package is missing payload root: " +
        payload_root.string());
  }

  std::vector<ImportedPayloadFile> imported_payloads;
  std::map<std::string, fs::path> source_by_month;
  for (const auto& entry : fs::recursive_directory_iterator(payload_root)) {
    if (!entry.is_regular_file() ||
        !HasExtensionCaseInsensitive(entry.path(), ".txt")) {
      continue;
    }

    const fs::path relative_path =
        entry.path().lexically_relative(extracted_root);
    const std::string relative_text = relative_path.generic_string();
    if (relative_text.empty()) {
      throw std::runtime_error("Failed to resolve imported payload path: " +
                               entry.path().string());
    }

    const ParsedMonthInfo file_info = ParseMonthInfoFromFileName(
        entry.path().filename().string(), entry.path().string());
    const ParsedMonthInfo header_info = ParseMonthInfoFromCanonicalText(
        ReadFileBytes(entry.path()), entry.path().string());
    if (file_info.month_key != header_info.month_key) {
      throw std::runtime_error(
          "Imported payload file name must match canonical month headers yYYYY + mMM: " +
          entry.path().string());
    }
    const std::string expected_relative =
        (fs::path(exchange_pkg::kPayloadRoot) / std::to_string(file_info.year) /
         file_info.file_name)
            .generic_string();
    if (relative_text != expected_relative) {
      throw std::runtime_error("Imported payload path must be exactly " +
                               expected_relative + ": " +
                               entry.path().string());
    }
    if (const auto [it, inserted] =
            source_by_month.emplace(file_info.month_key, entry.path());
        !inserted) {
      throw std::runtime_error(
          "Imported package contains duplicate month payload " +
          file_info.month_key + ": " + it->second.string() + " | " +
          entry.path().string());
    }

    imported_payloads.push_back(ImportedPayloadFile{
        .source_path = entry.path(),
        .relative_package_path = relative_text,
        .month_key = file_info.month_key,
        .year = file_info.year,
        .month = file_info.month,
    });
  }

  std::sort(imported_payloads.begin(), imported_payloads.end(),
            [](const ImportedPayloadFile& lhs,
               const ImportedPayloadFile& rhs) {
              return lhs.month_key < rhs.month_key;
            });
  return imported_payloads;
}

auto CollectManagedMonthFiles(const fs::path& active_text_root)
    -> std::map<std::string, fs::path> {
  std::map<std::string, fs::path> managed_month_files;
  if (active_text_root.empty() || !fs::exists(active_text_root)) {
    return managed_month_files;
  }
  if (!fs::is_directory(active_text_root)) {
    throw std::runtime_error("active_text_root_path must be a directory: " +
                             active_text_root.string());
  }

  for (const auto& entry : fs::recursive_directory_iterator(active_text_root)) {
    if (!entry.is_regular_file()) {
      continue;
    }
    if (!HasExtensionCaseInsensitive(entry.path(), ".txt")) {
      continue;
    }
    const ParsedMonthInfo month_info = ParseMonthInfoFromFileName(
        entry.path().filename().string(), entry.path().string());
    if (const auto [it, inserted] =
            managed_month_files.emplace(month_info.month_key, entry.path());
        !inserted) {
      throw std::runtime_error(
          "active_text_root_path contains duplicate month TXT files for " +
          month_info.month_key + ": " + it->second.string() + " | " +
          entry.path().string());
    }
  }
  return managed_month_files;
}
