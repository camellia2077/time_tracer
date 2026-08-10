class TracerCoreApi : public ITracerCoreApi {
 public:
  using ProjectRepositoryPtr = std::shared_ptr<IProjectRepository>;
  using DataQueryServicePtr =
      std::shared_ptr<tracer_core::application::ports::IDataQueryService>;
  using InsightsDataQueryServicePtr =
      std::shared_ptr<tracer_core::application::ports::IInsightsDataQueryService>;
  using InsightsDtoFormatterPtr =
      std::shared_ptr<tracer_core::application::ports::IInsightsDtoFormatter>;
  using TracerExchangeServicePtr =
      std::shared_ptr<tracer_core::application::ports::ITracerExchangeService>;

  TracerCoreApi(
      ::tracer::core::application::workflow::IWorkflowHandler& workflow_handler,
      IInsightsHandler& insights_handler, ProjectRepositoryPtr project_repository,
      DataQueryServicePtr data_query_service,
      InsightsDataQueryServicePtr insights_data_query_service = nullptr,
      InsightsDtoFormatterPtr insights_dto_formatter = nullptr,
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

  auto RunTemporalInsightsQuery(
      const tracer_core::core::dto::TemporalInsightsQueryRequest& request)
      -> tracer_core::core::dto::TextOutput override;

  auto RunTemporalStructuredInsightsQuery(
      const tracer_core::core::dto::TemporalStructuredInsightsQueryRequest&
          request)
      -> tracer_core::core::dto::TemporalStructuredInsightsOutput override;

  auto RunPeriodBatchQuery(
      const tracer_core::core::dto::PeriodBatchQueryRequest& request)
      -> tracer_core::core::dto::TextOutput override;

  auto RunStructuredPeriodBatchQuery(
      const tracer_core::core::dto::StructuredPeriodBatchQueryRequest& request)
      -> tracer_core::core::dto::StructuredPeriodBatchOutput override;

  auto RunTemporalInsightsTargetsQuery(
      const tracer_core::core::dto::TemporalInsightsTargetsRequest& request)
      -> tracer_core::core::dto::TemporalInsightsTargetsOutput override;

  auto RunTemporalInsightsExport(
      const tracer_core::core::dto::TemporalInsightsExportRequest& request)
      -> tracer_core::core::dto::OperationAck override;

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
  IInsightsHandler& insights_handler_;
  ProjectRepositoryPtr project_repository_;
  DataQueryServicePtr data_query_service_;
  InsightsDataQueryServicePtr kInsights_;
  InsightsDtoFormatterPtr insights_dto_formatter_;
  TracerExchangeServicePtr tracer_exchange_service_;
};
