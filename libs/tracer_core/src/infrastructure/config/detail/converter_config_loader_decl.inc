class ConverterConfigLoader {
 public:
  static auto LoadFromFile(const std::filesystem::path& main_config_path)
      -> ConverterConfig;

 private:
  static auto MergeTomlTable(toml::table& target, const toml::table& source)
      -> void;
  static auto MergeSectionIfPresent(toml::table& main_tbl,
                                    const toml::table& source_tbl,
                                    std::string_view section_key) -> void;
  static auto MergeOptionalSections(
      toml::table& main_tbl, const std::filesystem::path& config_dir,
      std::string_view path_key,
      std::initializer_list<std::string_view> section_keys) -> void;
  static auto LoadMergedToml(const std::filesystem::path& main_config_path)
      -> toml::table;
  static auto ParseTomlToStruct(const toml::table& tbl,
                                ConverterConfig& out_config) -> void;

  static auto ParseBasicConfig(const toml::table& tbl, ConverterConfig& config)
      -> void;
  static auto ParseGeneratedActivities(const toml::table& tbl,
                                       ConverterConfig& config) -> void;
  static auto ParseMappings(const toml::table& tbl, ConverterConfig& config)
      -> void;
  static auto ParseDurationMappings(const toml::table& tbl,
                                    ConverterConfig& config) -> void;
};
