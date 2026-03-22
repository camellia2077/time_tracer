class ConfigLoader {
 public:
  ConfigLoader(const std::string& exe_path_str);

  auto LoadConfiguration() -> AppConfig;
  [[nodiscard]] auto GetMainConfigPath() const -> std::string;

 private:
  std::filesystem::path exe_path_;
  std::filesystem::path config_dir_path_;
  std::filesystem::path main_config_path_;

  const std::string kConfigFileName = "config.toml";
  const std::string kConfigDirName = "config";
};
