class ITracerCoreApi {
 public:
  virtual ~ITracerCoreApi() = default;

  virtual auto RunConvert(const tracer_core::core::dto::ConvertRequest& request)
      -> tracer_core::core::dto::OperationAck = 0;

  virtual auto RunIngest(const tracer_core::core::dto::IngestRequest& request)
      -> tracer_core::core::dto::OperationAck = 0;

  virtual auto RunImport(const tracer_core::core::dto::ImportRequest& request)
      -> tracer_core::core::dto::OperationAck = 0;

  virtual auto RunValidateStructure(
      const tracer_core::core::dto::ValidateStructureRequest& request)
      -> tracer_core::core::dto::OperationAck = 0;

  virtual auto RunValidateLogic(
      const tracer_core::core::dto::ValidateLogicRequest& request)
      -> tracer_core::core::dto::OperationAck = 0;

  virtual auto RunReportQuery(
      const tracer_core::core::dto::ReportQueryRequest& request)
      -> tracer_core::core::dto::TextOutput = 0;

  virtual auto RunStructuredReportQuery(
      const tracer_core::core::dto::StructuredReportQueryRequest& request)
      -> tracer_core::core::dto::StructuredReportOutput = 0;

  virtual auto RunPeriodBatchQuery(
      const tracer_core::core::dto::PeriodBatchQueryRequest& request)
      -> tracer_core::core::dto::TextOutput = 0;

  virtual auto RunStructuredPeriodBatchQuery(
      const tracer_core::core::dto::StructuredPeriodBatchQueryRequest& request)
      -> tracer_core::core::dto::StructuredPeriodBatchOutput = 0;

  virtual auto RunReportExport(
      const tracer_core::core::dto::ReportExportRequest& request)
      -> tracer_core::core::dto::OperationAck = 0;

  virtual auto RunDataQuery(
      const tracer_core::core::dto::DataQueryRequest& request)
      -> tracer_core::core::dto::TextOutput = 0;

  virtual auto RunTreeQuery(
      const tracer_core::core::dto::TreeQueryRequest& request)
      -> tracer_core::core::dto::TreeQueryResponse = 0;
};
