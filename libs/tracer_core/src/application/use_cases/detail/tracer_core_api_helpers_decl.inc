auto BuildErrorMessage(std::string_view operation, std::string_view details)
    -> std::string;

auto BuildOperationFailure(std::string_view operation,
                           const std::exception& exception)
    -> tracer_core::core::dto::OperationAck;
auto BuildOperationFailure(std::string_view operation)
    -> tracer_core::core::dto::OperationAck;

auto BuildTextFailure(std::string_view operation,
                      const std::exception& exception)
    -> tracer_core::core::dto::TextOutput;
auto BuildTextFailure(std::string_view operation, std::string_view details)
    -> tracer_core::core::dto::TextOutput;
auto BuildTextFailure(std::string_view operation)
    -> tracer_core::core::dto::TextOutput;

auto BuildTreeFailure(std::string_view operation,
                      const std::exception& exception)
    -> tracer_core::core::dto::TreeQueryResponse;
auto BuildTreeFailure(std::string_view operation)
    -> tracer_core::core::dto::TreeQueryResponse;

auto BuildStructuredReportFailure(std::string_view operation,
                                  std::string_view details)
    -> tracer_core::core::dto::StructuredReportOutput;
auto BuildStructuredReportFailure(std::string_view operation,
                                  const std::exception& exception)
    -> tracer_core::core::dto::StructuredReportOutput;
auto BuildStructuredReportFailure(std::string_view operation)
    -> tracer_core::core::dto::StructuredReportOutput;

auto BuildStructuredPeriodBatchFailure(std::string_view operation,
                                       std::string_view details)
    -> tracer_core::core::dto::StructuredPeriodBatchOutput;
auto BuildStructuredPeriodBatchFailure(std::string_view operation,
                                       const std::exception& exception)
    -> tracer_core::core::dto::StructuredPeriodBatchOutput;
auto BuildStructuredPeriodBatchFailure(std::string_view operation)
    -> tracer_core::core::dto::StructuredPeriodBatchOutput;

auto FormatStructuredReport(
    const tracer_core::core::dto::StructuredReportOutput& output,
    ReportFormat format,
    tracer_core::application::ports::IReportDtoFormatter& formatter)
    -> tracer_core::core::dto::TextOutput;

auto BuildPeriodBatchErrorLine(int days, std::string_view details)
    -> std::string;
