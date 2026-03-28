class TracerCoreApi : public ITracerCoreApi {
 public:
  using ProjectRepositoryPtr = std::shared_ptr<IProjectRepository>;
  using DataQueryServicePtr =
      std::shared_ptr<tracer_core::application::ports::IDataQueryService>;
  using ReportDataQueryServicePtr =
      std::shared_ptr<tracer_core::application::ports::IReportDataQueryService>;
  using ReportDtoFormatterPtr =
      std::shared_ptr<tracer_core::application::ports::IReportDtoFormatter>;
  using TracerExchangeServicePtr =
      std::shared_ptr<tracer_core::application::ports::ITracerExchangeService>;

  TracerCoreApi(
      ::tracer::core::application::workflow::IWorkflowHandler& workflow_handler,
      IReportHandler& report_handler, ProjectRepositoryPtr project_repository,
      DataQueryServicePtr data_query_service,
      ReportDataQueryServicePtr report_data_query_service = nullptr,
      ReportDtoFormatterPtr report_dto_formatter = nullptr,
      TracerExchangeServicePtr tracer_exchange_service = nullptr);

  auto RunConvert(const tracer_core::core::dto::ConvertRequest& request)
      -> tracer_core::core::dto::OperationAck override;

  auto RunIngest(const tracer_core::core::dto::IngestRequest& request)
      -> tracer_core::core::dto::OperationAck override;

  auto RunImport(const tracer_core::core::dto::ImportRequest& request)
      -> tracer_core::core::dto::OperationAck override;

  auto RunValidateStructure(
      const tracer_core::core::dto::ValidateStructureRequest& request)
      -> tracer_core::core::dto::OperationAck override;

  auto RunValidateLogic(
      const tracer_core::core::dto::ValidateLogicRequest& request)
      -> tracer_core::core::dto::OperationAck override;

  auto RunReportQuery(const tracer_core::core::dto::ReportQueryRequest& request)
      -> tracer_core::core::dto::TextOutput override;

  auto RunStructuredReportQuery(
      const tracer_core::core::dto::StructuredReportQueryRequest& request)
      -> tracer_core::core::dto::StructuredReportOutput override;

  auto RunPeriodBatchQuery(
      const tracer_core::core::dto::PeriodBatchQueryRequest& request)
      -> tracer_core::core::dto::TextOutput override;

  auto RunStructuredPeriodBatchQuery(
      const tracer_core::core::dto::StructuredPeriodBatchQueryRequest& request)
      -> tracer_core::core::dto::StructuredPeriodBatchOutput override;

  auto RunDataQuery(const tracer_core::core::dto::DataQueryRequest& request)
      -> tracer_core::core::dto::TextOutput override;

  auto RunTreeQuery(const tracer_core::core::dto::TreeQueryRequest& request)
      -> tracer_core::core::dto::TreeQueryResponse override;

  auto RunTracerExchangeExport(
      const tracer_core::core::dto::TracerExchangeExportRequest& request)
      -> tracer_core::core::dto::TracerExchangeExportResult override;

  auto RunTracerExchangeImport(
      const tracer_core::core::dto::TracerExchangeImportRequest& request)
      -> tracer_core::core::dto::TracerExchangeImportResult override;

  auto RunTracerExchangeInspect(
      const tracer_core::core::dto::TracerExchangeInspectRequest& request)
      -> tracer_core::core::dto::TracerExchangeInspectResult override;

 private:
  ::tracer::core::application::workflow::IWorkflowHandler& workflow_handler_;
  IReportHandler& report_handler_;
  ProjectRepositoryPtr project_repository_;
  DataQueryServicePtr data_query_service_;
  ReportDataQueryServicePtr kReport_;
  ReportDtoFormatterPtr report_dto_formatter_;
  TracerExchangeServicePtr tracer_exchange_service_;
};
