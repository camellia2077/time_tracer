class ConverterConfigLoader {
 public:
  static auto LoadFromFile(const std::filesystem::path& main_config_path)
      -> ConverterConfig;

 private:
  static auto LoadMergedToml(const std::filesystem::path& main_config_path)
      -> toml::table;
  static auto ParseTomlToStruct(const toml::table& tbl,
                                ConverterConfig& out_config) -> void;

  static auto ParseSleepInference(const toml::table& tbl,
                                  ConverterConfig& config) -> void;
  static auto ParseMappings(const toml::table& tbl, ConverterConfig& config)
      -> void;
};
