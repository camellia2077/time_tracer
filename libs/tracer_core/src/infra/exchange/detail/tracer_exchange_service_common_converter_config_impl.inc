auto ResolveActiveConverterConfigPaths(
    const fs::path& active_converter_main_config_path)
    -> ActiveConverterConfigPaths {
  if (active_converter_main_config_path.empty()) {
    throw std::invalid_argument(
        "active_converter_main_config_path must not be empty.");
  }
  const fs::path main_config_path =
      fs::absolute(active_converter_main_config_path);
  const fs::path alias_directory_path =
      main_config_path.parent_path() / "activity_hierarchy";
  const fs::path config_root = main_config_path.parent_path().parent_path();
  const auto alias_definition =
      tracer::core::infrastructure::config::loader::detail::
          LoadAliasMappingDefinition(
              alias_directory_path,
              tracer::core::infrastructure::config::loader::ReadToml);

  ActiveConverterConfigPaths paths{
      .config_root_path = config_root,
      .main_config_path = main_config_path,
      .alias_directory_path = alias_directory_path,
  };
  paths.alias_child_paths.reserve(alias_definition.child_files.size());
  for (const auto& child_file : alias_definition.child_files) {
    paths.alias_child_paths.push_back(child_file.absolute_path);
  }
  return paths;
}
auto EnsureActiveConverterConfigExists(
    const ActiveConverterConfigPaths& active_paths) -> void {
  EnsureRegularFileExists(active_paths.main_config_path,
                          "Active converter main config");
  if (!fs::exists(active_paths.alias_directory_path) ||
      !fs::is_directory(active_paths.alias_directory_path)) {
    throw std::runtime_error("Active activity hierarchy directory not found: " +
                             active_paths.alias_directory_path.string());
  }
  for (const auto& child_path : active_paths.alias_child_paths) {
    EnsureRegularFileExists(child_path, "Active activity hierarchy TOML");
  }
}
auto BackupActiveConverterConfig(const ActiveConverterConfigPaths& active_paths,
                                 const fs::path& backup_root) -> void {
  EnsureDirectoryRemoved(backup_root);
  EnsureActiveConverterConfigExists(active_paths);
  CopyFileWithOverwrite(
      active_paths.main_config_path,
      BuildBackupPath(backup_root, "config/user/behavior.toml"));
  for (const auto& child_path : active_paths.alias_child_paths) {
    const fs::path relative_path =
        fs::relative(child_path, active_paths.alias_directory_path.parent_path());
    CopyFileWithOverwrite(child_path,
                          BuildBackupPath(backup_root,
                                          (fs::path("config") / relative_path)
                                              .generic_string()));
  }
}
