namespace tracer::core::application::workflow {

auto WorkflowHandler::RunIngest(const std::string& source_path,
                                DateCheckMode date_check_mode,
                                bool save_processed, IngestMode ingest_mode)
    -> void {
  app_ports::LogInfo("\n--- 启动数据摄入 (Ingest) ---");
  modports::ClearBufferedDiagnostics();

  const auto kDbCheck = database_health_checker_->CheckReady();
  if (!kDbCheck.ok) {
    throw std::runtime_error(kDbCheck.message.empty()
                                 ? "Database readiness check failed."
                                 : kDbCheck.message);
  }

  PipelineOrchestrator pipeline(output_root_path_, converter_config_provider_,
                                ingest_input_provider_,
                                processed_data_storage_,
                                validation_issue_reporter_);
  const AppOptions kFullOptions =
      BuildIngestOptions(source_path, date_check_mode, save_processed);
  auto result_context_opt = pipeline.Run(kFullOptions);

  if (!result_context_opt) {
    app_ports::LogError("\n=== Ingest 执行失败 ===");
    throw std::runtime_error(
        workflow_handler_internal::BuildPipelineFailureMessage(
            "Ingestion process failed."));
  }

  auto& context = *result_context_opt;
  app_ports::LogInfo("\n--- 流水线验证通过，准备入库 ---");

  if (ingest_mode == IngestMode::kSingleTxtReplaceMonth) {
    const auto kTargetMonth = TryResolveSingleTxtTargetMonth(context);
    if (!kTargetMonth.has_value()) {
      throw std::runtime_error(
          "Single TXT replace-month ingest requires exactly one TXT input "
          "with valid headers: yYYYY + mMM.");
    }

    if (!IsSingleMonthConsistent(context.result.processed_data,
                                 kTargetMonth->month_key)) {
      throw std::runtime_error(
          "Single TXT replace-month ingest failed: parsed days are not "
          "consistent with header month " +
          kTargetMonth->month_key + ".");
    }

    const auto kPreviousTail = ResolvePreviousTailForReplaceMonth(
        context, *kTargetMonth, context.result.processed_data,
        *time_sheet_repository_);
    if (kPreviousTail.has_value()) {
      LogLinker linker(context.state.converter_config);
      linker.LinkFirstDayWithExternalPreviousEvent(
          context.result.processed_data,
          LogLinker::ExternalPreviousEvent{
              .date = kPreviousTail->date,
              .end_time = kPreviousTail->end_time,
          });
    } else if (!context.result.processed_data.empty()) {
      app_ports::LogWarn(
          "[LogLinker] No previous-month tail context found (DB/sibling "
          "TXT). Ingest will proceed without cross-month backfill.");
    }

    RunDatabaseImportFromMemoryReplacingMonth(
        context.result.processed_data, kTargetMonth->year, kTargetMonth->month);
    app_ports::LogInfo("\n=== Ingest 执行成功（单月替换）===");
    return;
  }

  if (!context.result.processed_data.empty()) {
    RunDatabaseImportFromMemory(context.result.processed_data);
    app_ports::LogInfo("\n=== Ingest 执行成功 ===");
  } else {
    app_ports::LogWarn("\n=== Ingest 完成但无数据产生 ===");
  }
}

auto WorkflowHandler::RunIngestReplacingAll(const std::string& source_path,
                                            DateCheckMode date_check_mode,
                                            bool save_processed) -> void {
  app_ports::LogInfo("\n--- 启动数据摄入 (Replace All) ---");
  modports::ClearBufferedDiagnostics();

  const auto db_check = database_health_checker_->CheckReady();
  if (!db_check.ok) {
    throw std::runtime_error(db_check.message.empty()
                                 ? "Database readiness check failed."
                                 : db_check.message);
  }

  PipelineOrchestrator pipeline(output_root_path_, converter_config_provider_,
                                ingest_input_provider_,
                                processed_data_storage_,
                                validation_issue_reporter_);
  const AppOptions full_options =
      BuildIngestOptions(source_path, date_check_mode, save_processed);
  auto result_context_opt = pipeline.Run(full_options);

  if (!result_context_opt) {
    app_ports::LogError("\n=== Replace-all ingest 执行失败 ===");
    throw std::runtime_error(
        workflow_handler_internal::BuildPipelineFailureMessage(
            "Replace-all ingestion process failed."));
  }

  auto& context = *result_context_opt;
  app_ports::LogInfo("\n--- 流水线验证通过，准备全量替换入库 ---");
  RunDatabaseImportFromMemoryReplacingAll(context.result.processed_data);
  app_ports::LogInfo("\n=== Ingest 执行成功（全量替换）===");
}

}  // namespace tracer::core::application::workflow
