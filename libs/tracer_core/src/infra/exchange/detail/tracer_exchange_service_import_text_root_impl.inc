auto BuildManagedMonthRelativePath(int year, std::string_view month_key)
    -> fs::path {
  return fs::path(std::to_string(year)) /
         fs::path(std::string(month_key) + ".txt");
}

auto ResolveManagedTextRelativePath(const fs::path& active_text_root,
                                    const fs::path& source_path) -> fs::path {
  const fs::path relative_path =
      source_path.lexically_relative(active_text_root);
  if (relative_path.empty() || relative_path == source_path) {
    throw std::runtime_error("Managed text file must stay under active text "
                             "root: " +
                             source_path.string());
  }
  return relative_path;
}

auto ResolveImportedPayloadTargetRelativePath(
    const ImportedPayloadFile& imported_payload) -> fs::path {
  return BuildManagedMonthRelativePath(imported_payload.year,
                                       imported_payload.month_key);
}

auto BackupManagedTextFiles(
    const fs::path& active_text_root,
    const std::vector<ImportedPayloadFile>& imported_payloads,
    const std::map<std::string, fs::path>& managed_month_files,
    const fs::path& backup_root) -> void {
  for (const auto& imported_payload : imported_payloads) {
    const auto existing_it =
        managed_month_files.find(imported_payload.month_key);
    if (existing_it == managed_month_files.end()) {
      continue;
    }

    const fs::path existing_relative_path =
        ResolveManagedTextRelativePath(active_text_root, existing_it->second);
    const fs::path backup_path = backup_root / existing_relative_path;
    EnsureParentDirectory(backup_path);
    std::error_code error;
    fs::copy_file(existing_it->second, backup_path,
                  fs::copy_options::overwrite_existing, error);
    if (error) {
      throw std::runtime_error(
          "Failed to backup managed text file: " + existing_it->second.string() +
          " -> " + backup_path.string() + " | " + error.message());
    }
  }
}

auto RestoreManagedTextFiles(
    const fs::path& active_text_root, const fs::path& backup_root,
    const std::vector<ImportedPayloadFile>& imported_payloads) -> void {
  for (const auto& imported_payload : imported_payloads) {
    const fs::path imported_target =
        active_text_root / ResolveImportedPayloadTargetRelativePath(imported_payload);
    std::error_code remove_error;
    fs::remove(imported_target, remove_error);
    if (remove_error) {
      throw std::runtime_error("Failed to remove imported month TXT during "
                               "rollback: " +
                               imported_target.string() + " | " +
                               remove_error.message());
    }
  }

  if (!fs::exists(backup_root)) {
    return;
  }

  for (const auto& entry : fs::recursive_directory_iterator(backup_root)) {
    if (!entry.is_regular_file()) {
      continue;
    }
    const fs::path relative_path =
        entry.path().lexically_relative(backup_root);
    const fs::path restore_path = active_text_root / relative_path;
    EnsureParentDirectory(restore_path);
    std::error_code error;
    fs::copy_file(entry.path(), restore_path,
                  fs::copy_options::overwrite_existing, error);
    if (error) {
      throw std::runtime_error("Failed to restore managed text file: " +
                               entry.path().string() + " -> " +
                               restore_path.string() + " | " +
                               error.message());
    }
  }
}

auto BuildEffectiveTextRoot(
    const std::map<std::string, fs::path>& managed_month_files,
    const std::vector<ImportedPayloadFile>& imported_payloads,
    const fs::path& effective_root) -> void {
  RemoveDirectoryBestEffort(effective_root);
  std::error_code error;
  fs::create_directories(effective_root, error);
  if (error) {
    throw std::runtime_error("Failed to create effective text root: " +
                             effective_root.string() + " | " + error.message());
  }

  for (const auto& [month_key, source_path] : managed_month_files) {
    const ParsedMonthInfo month_info = ParseMonthInfoFromFileName(
        source_path.filename().string(), source_path.string());
    const fs::path target_relative =
        BuildManagedMonthRelativePath(month_info.year, month_key);
    EnsureParentDirectory(effective_root / target_relative);
    std::error_code copy_error;
    fs::copy_file(source_path, effective_root / target_relative,
                  fs::copy_options::overwrite_existing, copy_error);
    if (copy_error) {
      throw std::runtime_error(
          "Failed to seed effective text root from active text root: " +
          source_path.string() + " | " + copy_error.message());
    }
  }

  for (const auto& imported_payload : imported_payloads) {
    const fs::path target_relative =
        ResolveImportedPayloadTargetRelativePath(imported_payload);
    EnsureParentDirectory(effective_root / target_relative);
    std::error_code copy_error;
    fs::copy_file(imported_payload.source_path, effective_root / target_relative,
                  fs::copy_options::overwrite_existing, copy_error);
    if (copy_error) {
      throw std::runtime_error(
          "Failed to write imported payload into effective text root: " +
          imported_payload.source_path.string() + " | " + copy_error.message());
    }
  }
}

auto WriteImportedPayloadsToActiveTextRoot(
    const fs::path& active_text_root,
    const std::map<std::string, fs::path>& managed_month_files,
    const std::vector<ImportedPayloadFile>& imported_payloads) -> void {
  std::error_code error;
  fs::create_directories(active_text_root, error);
  if (error) {
    throw std::runtime_error(
        "Failed to create active_text_root_path: " + active_text_root.string() +
        " | " + error.message());
  }

  for (const auto& imported_payload : imported_payloads) {
    const fs::path target =
        active_text_root / ResolveImportedPayloadTargetRelativePath(imported_payload);
    EnsureParentDirectory(target);
    std::error_code copy_error;
    fs::copy_file(imported_payload.source_path, target,
                  fs::copy_options::overwrite_existing, copy_error);
    if (copy_error) {
      throw std::runtime_error(
          "Failed to update active text root month file: " +
          imported_payload.source_path.string() + " -> " + target.string() +
          " | " + copy_error.message());
    }

    const auto existing_it =
        managed_month_files.find(imported_payload.month_key);
    if (existing_it == managed_month_files.end()) {
      continue;
    }

    std::error_code equivalent_error;
    const bool same_target =
        fs::equivalent(existing_it->second, target, equivalent_error);
    if (equivalent_error) {
      throw std::runtime_error(
          "Failed to compare managed text file paths after import: " +
          existing_it->second.string() + " | " + target.string() + " | " +
          equivalent_error.message());
    }
    if (same_target) {
      continue;
    }
    std::error_code remove_error;
    fs::remove(existing_it->second, remove_error);
    if (remove_error) {
      throw std::runtime_error(
          "Failed to remove legacy managed text file after import: " +
          existing_it->second.string() + " | " + remove_error.message());
    }
  }
}
