auto CopyFileWithOverwrite(const fs::path& source, const fs::path& target)
    -> void {
  EnsureParentDirectory(target);
  std::error_code error;
  fs::copy_file(source, target, fs::copy_options::overwrite_existing, error);
  if (error) {
    throw std::runtime_error("Failed to copy file: " + source.string() +
                             " -> " + target.string() + " | " +
                             error.message());
  }
}

auto BuildBackupPath(const fs::path& backup_root,
                     std::string_view relative_path) -> fs::path {
  return backup_root / fs::path(relative_path);
}
