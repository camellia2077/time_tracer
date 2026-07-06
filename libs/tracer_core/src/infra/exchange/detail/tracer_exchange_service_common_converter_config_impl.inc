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
  const fs::path config_root = converter_dir.parent_path();
  const fs::path alias_mapping_path = converter_dir / "alias_mapping.toml";
  const auto alias_index_tbl =
      tracer::core::infrastructure::config::loader::ReadToml(alias_mapping_path);
  const auto alias_definition =
      tracer::core::infrastructure::config::loader::detail::
          LoadAliasMappingDefinition(
              alias_mapping_path, alias_index_tbl,
              tracer::core::infrastructure::config::loader::ReadToml);

  ActiveConverterConfigPaths paths{
      .config_root_path = config_root,
      .main_config_path = main_config_path,
      .alias_mapping_path = alias_mapping_path,
      .duration_rules_path = converter_dir / "duration_rules.toml",
  };
  paths.alias_child_paths.reserve(alias_definition.child_files.size());
  for (const auto& child_file : alias_definition.child_files) {
    paths.alias_child_paths.push_back(child_file.absolute_path);
  }
  paths.report_markdown_paths.reserve(
      exchange_pkg::kReportMarkdownPackagePaths.size());
  for (const auto report_path : exchange_pkg::kReportMarkdownPackagePaths) {
    const fs::path config_relative = fs::path(report_path).lexically_relative(
        fs::path("config"));
    paths.report_markdown_paths.push_back(config_root / config_relative);
  }
  return paths;
}

auto EnsureActiveConverterConfigExists(
    const ActiveConverterConfigPaths& active_paths) -> void {
  EnsureRegularFileExists(active_paths.main_config_path,
                          "Active converter main config");
  EnsureRegularFileExists(active_paths.alias_mapping_path,
                          "Active alias mapping config");
  EnsureRegularFileExists(active_paths.duration_rules_path,
                          "Active duration rules config");
  for (const auto& child_path : active_paths.alias_child_paths) {
    EnsureRegularFileExists(child_path, "Active alias child config");
  }
  for (const auto& report_path : active_paths.report_markdown_paths) {
    EnsureRegularFileExists(report_path, "Active markdown report config");
  }
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
      BuildBackupPath(backup_root, exchange_pkg::kAliasMappingIndexPath));
  CopyFileWithOverwrite(
      active_paths.duration_rules_path,
      BuildBackupPath(backup_root, exchange_pkg::kDurationRulesPath));
  for (const auto& child_path : active_paths.alias_child_paths) {
    const fs::path relative_path =
        fs::relative(child_path, active_paths.alias_mapping_path.parent_path());
    CopyFileWithOverwrite(child_path,
                          BuildBackupPath(backup_root,
                                          (fs::path("config") / "converter" /
                                           relative_path)
                                              .generic_string()));
  }
  for (std::size_t index = 0; index < active_paths.report_markdown_paths.size();
       ++index) {
    CopyFileWithOverwrite(
        active_paths.report_markdown_paths[index],
        BuildBackupPath(backup_root,
                        exchange_pkg::kReportMarkdownPackagePaths[index]));
  }
}

auto ValidatePackageConverterConfig(const fs::path& work_root) -> void {
  const fs::path main_config_path =
      work_root / fs::path(exchange_pkg::kConverterMainPath);
  EnsureRegularFileExists(main_config_path, "Package converter main config");
  static_cast<void>(
      infra_config::ConverterConfigLoader::LoadFromFile(main_config_path));
}

auto InstallPackageMarkdownReportConfig(const fs::path& package_root,
                                        const fs::path& active_config_root)
    -> void {
  for (const auto report_path : exchange_pkg::kReportMarkdownPackagePaths) {
    const fs::path package_source = package_root / fs::path(report_path);
    const fs::path config_relative =
        fs::path(report_path).lexically_relative(fs::path("config"));
    const fs::path active_target = active_config_root / config_relative;
    EnsureRegularFileExists(package_source, "Package markdown report config");
    CopyFileWithOverwrite(package_source, active_target);
  }
}

auto RestoreMarkdownReportConfig(const fs::path& backup_root,
                                 const fs::path& active_config_root) -> void {
  InstallPackageMarkdownReportConfig(backup_root, active_config_root);
}
