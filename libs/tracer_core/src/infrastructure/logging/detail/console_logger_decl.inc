class ConsoleLogger final : public tracer_core::application::ports::ILogger {
 public:
  auto Log(tracer_core::application::ports::LogSeverity severity,
           std::string_view message) -> void override;

 private:
  std::mutex output_mutex_;
};
