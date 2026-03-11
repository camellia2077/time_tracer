class ValidationIssueReporter final
    : public tracer_core::application::ports::IValidationIssueReporter {
 public:
  auto ReportStructureErrors(std::string_view display_label,
                             const std::set<validator::Error>& errors)
      -> void override;

  auto ReportLogicDiagnostics(
      std::string_view fallback_label,
      const std::vector<validator::Diagnostic>& diagnostics) -> void override;
};
