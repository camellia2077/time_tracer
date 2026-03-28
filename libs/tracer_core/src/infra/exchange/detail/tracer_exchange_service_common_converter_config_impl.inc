auto ResolveActiveConverterConfigPaths(
    const fs::path& active_converter_main_config_path)
    -> ActiveConverterConfigPaths {
  if (active_converter_main_config_path.empty()) {
    throw std::invalid_argument(
        "active_converter_main_config_path must not be empty.");
  }
  const fs::path main_config_path =
      fs::absolute(active_converter_main_config_path);
  const fs::path converter_dir = main_config_path.parent_path();
  return {
      .main_config_path = main_config_path,
      .alias_mapping_path = converter_dir / "alias_mapping.toml",
      .duration_rules_path = converter_dir / "duration_rules.toml",
  };
}

auto EnsureActiveConverterConfigExists(
    const ActiveConverterConfigPaths& active_paths) -> void {
  EnsureRegularFileExists(active_paths.main_config_path,
                          "Active converter main config");
  EnsureRegularFileExists(active_paths.alias_mapping_path,
                          "Active alias mapping config");
  EnsureRegularFileExists(active_paths.duration_rules_path,
                          "Active duration rules config");
}

auto BackupActiveConverterConfig(const ActiveConverterConfigPaths& active_paths,
                                 const fs::path& backup_root) -> void {
  EnsureDirectoryRemoved(backup_root);
  EnsureActiveConverterConfigExists(active_paths);
  CopyFileWithOverwrite(
      active_paths.main_config_path,
      BuildBackupPath(backup_root, exchange_pkg::kConverterMainPath));
  CopyFileWithOverwrite(
      active_paths.alias_mapping_path,
      BuildBackupPath(backup_root, exchange_pkg::kAliasMappingPath));
  CopyFileWithOverwrite(
      active_paths.duration_rules_path,
      BuildBackupPath(backup_root, exchange_pkg::kDurationRulesPath));
}

auto ValidatePackageConverterConfig(const fs::path& work_root) -> void {
  const fs::path main_config_path =
      work_root / fs::path(exchange_pkg::kConverterMainPath);
  EnsureRegularFileExists(main_config_path, "Package converter main config");
  static_cast<void>(
      infra_config::ConverterConfigLoader::LoadFromFile(main_config_path));
}
