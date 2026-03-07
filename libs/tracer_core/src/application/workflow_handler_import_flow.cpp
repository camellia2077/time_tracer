// application/workflow_handler_import_flow.cpp
#include <algorithm>
#include <cctype>
#include <format>
#include <fstream>
#include <optional>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "application/pipeline/pipeline_manager.hpp"
#include "application/ports/logger.hpp"
#include "application/ports/i_database_health_checker.hpp"
#include "application/ports/i_time_sheet_repository.hpp"
#include "application/workflow_handler.hpp"
#include "domain/logic/converter/convert/core/converter_core.hpp"
#include "domain/logic/converter/log_processor.hpp"
#include "domain/ports/diagnostics.hpp"
#include "domain/types/app_options.hpp"
#include "shared/types/ansi_colors.hpp"
#include "shared/utils/string_utils.hpp"

using namespace core::pipeline;
namespace app_ports = tracer_core::application::ports;

namespace workflow_handler_internal {
[[nodiscard]] auto BuildPipelineFailureMessage(std::string_view base_message)
    -> std::string;
}  // namespace workflow_handler_internal

#include "application/internal/workflow_handler_import_namespace.inc"


auto WorkflowHandler::RunIngest(const std::string& source_path,
                                DateCheckMode date_check_mode,
                                bool save_processed, IngestMode ingest_mode)
    -> void {
  app_ports::LogInfo("\n--- 启动数据摄入 (Ingest) ---");
  tracer_core::domain::ports::ClearBufferedDiagnostics();

  const auto kDbCheck = database_health_checker_->CheckReady();
  if (!kDbCheck.ok) {
    throw std::runtime_error(kDbCheck.message.empty()
                                 ? "Database readiness check failed."
                                 : kDbCheck.message);
  }

  PipelineManager pipeline(output_root_path_, converter_config_provider_,
                           ingest_input_provider_, processed_data_storage_,
                           validation_issue_reporter_);
  const AppOptions kFullOptions =
      BuildIngestOptions(date_check_mode, save_processed);
  auto result_context_opt = pipeline.Run(source_path, kFullOptions);

  namespace colors = tracer_core::common::colors;
  if (!result_context_opt) {
    app_ports::LogError(std::string(colors::kRed) +
                        "\n=== Ingest 执行失败 ===" + colors::kReset.data());
    throw std::runtime_error(
        workflow_handler_internal::BuildPipelineFailureMessage(
            "Ingestion process failed."));
  }

  // Persistence gate: reaching this point means the ingest pipeline has
  // already completed conversion, structure validation, and logic validation.
  // Any database creation or write-side effect must happen only after this
  // boundary.
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
    app_ports::LogInfo(
        std::string(colors::kGreen) +
        "\n=== Ingest 执行成功（单月替换）===" + colors::kReset.data());
    return;
  }

  if (!context.result.processed_data.empty()) {
    RunDatabaseImportFromMemory(context.result.processed_data);
    app_ports::LogInfo(std::string(colors::kGreen) +
                       "\n=== Ingest 执行成功 ===" + colors::kReset.data());
  } else {
    app_ports::LogWarn(
        std::string(colors::kYellow) +
        "\n=== Ingest 完成但无数据产生 ===" + colors::kReset.data());
  }
}
