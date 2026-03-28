class StaticConverterConfigProvider final
    : public tracer_core::application::ports::IConverterConfigProvider {
 public:
  explicit StaticConverterConfigProvider(ConverterConfig converter_config);

  [[nodiscard]] auto LoadConverterConfig() const -> ConverterConfig override;
  auto InvalidateCache() -> void override;

 private:
  ConverterConfig converter_config_;
};
