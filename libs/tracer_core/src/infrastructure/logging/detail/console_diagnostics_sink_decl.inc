class ConsoleDiagnosticsSink final
    : public tracer_core::domain::ports::IDiagnosticsSink {
 public:
  auto Emit(tracer_core::domain::ports::DiagnosticSeverity severity,
            std::string_view message) -> void override;
};
