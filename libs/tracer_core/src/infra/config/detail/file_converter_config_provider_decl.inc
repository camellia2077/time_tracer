class FileConverterConfigProvider final
    : public tracer_core::application::ports::IConverterConfigProvider {
 public:
  FileConverterConfigProvider(
      std::filesystem::path config_path,
      std::unordered_map<std::filesystem::path, std::filesystem::path>
          initial_top_parents);

  [[nodiscard]] auto LoadConverterConfig() const -> ConverterConfig override;
  auto InvalidateCache() -> void override;

 private:
  std::filesystem::path config_path_;
  std::unordered_map<std::filesystem::path, std::filesystem::path>
      initial_top_parents_;
  mutable std::optional<ConverterConfig> cached_config_;
};
