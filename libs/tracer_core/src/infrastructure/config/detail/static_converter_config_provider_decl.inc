class StaticConverterConfigProvider final
    : public tracer_core::application::ports::IConverterConfigProvider {
 public:
  explicit StaticConverterConfigProvider(ConverterConfig converter_config);

  [[nodiscard]] auto LoadConverterConfig() const -> ConverterConfig override;

 private:
  ConverterConfig converter_config_;
};
