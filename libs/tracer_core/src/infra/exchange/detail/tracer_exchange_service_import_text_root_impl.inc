auto BackupManagedTextFiles(
    const std::vector<ImportedPayloadFile>& imported_payloads,
    const std::map<std::string, fs::path>& managed_month_files,
    const fs::path& backup_root) -> std::vector<std::string> {
  std::vector<std::string> added_month_keys;
  for (const auto& imported_payload : imported_payloads) {
    const auto existing_it = managed_month_files.find(imported_payload.month_key);
    if (existing_it == managed_month_files.end()) {
      added_month_keys.push_back(imported_payload.month_key);
      continue;
    }

    const fs::path backup_path =
        backup_root / fs::path(imported_payload.month_key + ".txt");
    EnsureParentDirectory(backup_path);
    std::error_code error;
    fs::copy_file(existing_it->second, backup_path,
                  fs::copy_options::overwrite_existing, error);
    if (error) {
      throw std::runtime_error("Failed to backup managed text file: " +
                               existing_it->second.string() + " -> " +
                               backup_path.string() + " | " +
                               error.message());
    }
  }

  std::sort(added_month_keys.begin(), added_month_keys.end());
  return added_month_keys;
}

auto RestoreManagedTextFiles(const fs::path& active_text_root,
                             const fs::path& backup_root,
                             const std::vector<std::string>& added_month_keys)
    -> void {
  if (fs::exists(backup_root)) {
    for (const auto& entry : fs::directory_iterator(backup_root)) {
      if (!entry.is_regular_file()) {
        continue;
      }
      EnsureParentDirectory(active_text_root / entry.path().filename());
      std::error_code error;
      fs::copy_file(entry.path(), active_text_root / entry.path().filename(),
                    fs::copy_options::overwrite_existing, error);
      if (error) {
        throw std::runtime_error("Failed to restore managed text file: " +
                                 entry.path().string() + " | " +
                                 error.message());
      }
    }
  }

  for (const auto& added_month_key : added_month_keys) {
    std::error_code error;
    fs::remove(active_text_root / fs::path(added_month_key + ".txt"), error);
    if (error) {
      throw std::runtime_error(
          "Failed to remove newly added month TXT during rollback: " +
          (active_text_root / fs::path(added_month_key + ".txt")).string() +
          " | " + error.message());
    }
  }
}

auto BuildEffectiveTextRoot(
    const fs::path& active_text_root,
    const std::map<std::string, fs::path>& managed_month_files,
    const std::vector<ImportedPayloadFile>& imported_payloads,
    const fs::path& effective_root) -> void {
  RemoveDirectoryBestEffort(effective_root);
  std::error_code error;
  fs::create_directories(effective_root, error);
  if (error) {
    throw std::runtime_error("Failed to create effective text root: " +
                             effective_root.string() + " | " +
                             error.message());
  }

  for (const auto& [month_key, source_path] : managed_month_files) {
    EnsureParentDirectory(effective_root / fs::path(month_key + ".txt"));
    std::error_code copy_error;
    fs::copy_file(source_path, effective_root / fs::path(month_key + ".txt"),
                  fs::copy_options::overwrite_existing, copy_error);
    if (copy_error) {
      throw std::runtime_error(
          "Failed to seed effective text root from active text root: " +
          source_path.string() + " | " + copy_error.message());
    }
  }

  for (const auto& imported_payload : imported_payloads) {
    EnsureParentDirectory(
        effective_root / fs::path(imported_payload.month_key + ".txt"));
    std::error_code copy_error;
    fs::copy_file(
        imported_payload.source_path,
        effective_root / fs::path(imported_payload.month_key + ".txt"),
        fs::copy_options::overwrite_existing, copy_error);
    if (copy_error) {
      throw std::runtime_error(
          "Failed to write imported payload into effective text root: " +
          imported_payload.source_path.string() + " | " +
          copy_error.message());
    }
  }
}

auto WriteImportedPayloadsToActiveTextRoot(
    const fs::path& active_text_root,
    const std::vector<ImportedPayloadFile>& imported_payloads) -> void {
  std::error_code error;
  fs::create_directories(active_text_root, error);
  if (error) {
    throw std::runtime_error("Failed to create active_text_root_path: " +
                             active_text_root.string() + " | " +
                             error.message());
  }

  for (const auto& imported_payload : imported_payloads) {
    const fs::path target =
        active_text_root / fs::path(imported_payload.month_key + ".txt");
    EnsureParentDirectory(target);
    std::error_code copy_error;
    fs::copy_file(imported_payload.source_path, target,
                  fs::copy_options::overwrite_existing, copy_error);
    if (copy_error) {
      throw std::runtime_error("Failed to update active text root month file: " +
                               imported_payload.source_path.string() + " -> " +
                               target.string() + " | " +
                               copy_error.message());
    }
  }
}
