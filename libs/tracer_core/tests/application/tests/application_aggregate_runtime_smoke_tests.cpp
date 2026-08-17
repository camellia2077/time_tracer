import tracer.core.application;
import tracer.core.domain.model.daily_log;
import tracer.core.domain.repositories.project_repository;
import tracer.core.domain.types.app_options;
import tracer.core.domain.types.date_check_mode;
import tracer.core.domain.types.ingest_mode;

#include "application/aggregate_runtime/tracer_core_runtime.hpp"
#include "application/compat/insights/i_insights_handler.hpp"
#include "application/dto/query_requests.hpp"
#include "application/dto/insights_requests.hpp"
#include "application/dto/shared_envelopes.hpp"
#include "application/pipeline/i_pipeline_workflow.hpp"
#include "application/ports/query/i_data_query_service.hpp"
#include "application/ports/insights/i_insights_data_query_service.hpp"
#include "application/ports/insights/i_insights_dto_formatter.hpp"

#include <exception>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace {

using tracer::core::domain::modmodel::DailyLog;
using tracer::core::domain::modrepos::IProjectRepository;
using tracer::core::domain::modrepos::ProjectEntity;
using tracer::core::domain::modtypes::AppOptions;
using tracer::core::domain::modtypes::DateCheckMode;
using tracer::core::domain::modtypes::IngestMode;

namespace app_use_cases = tracer::core::application::use_cases;

class SmokePipelineWorkflow final
    : public tracer::core::application::workflow::IWorkflowHandler {
 public:
  auto RunConverter(const std::string&, const AppOptions&) -> void override {}
  auto RunDatabaseImport(const std::string&) -> void override {}
  auto RunDatabaseImportFromMemory(
      const std::map<std::string, std::vector<DailyLog>>&) -> void override {}
  auto RunIngest(const std::string&, DateCheckMode, bool, IngestMode)
      -> void override {}
  auto RunIngestSyncStatusQuery(
      const tracer_core::core::dto::IngestSyncStatusRequest&)
      -> tracer_core::core::dto::IngestSyncStatusOutput override {
    return {.ok = true, .items = {}, .error_message = ""};
  }
  auto ClearIngestSyncStatus() -> void override {}
  auto RunIngestReplacingAll(const std::string&, DateCheckMode, bool)
      -> void override {}
  auto RunValidateStructure(const std::string&) -> void override {}
  auto RunValidateLogic(const std::string&, DateCheckMode) -> void override {}
  auto RunRecordActivityAtomically(
      const tracer_core::core::dto::RecordActivityAtomicallyRequest&)
      -> tracer_core::core::dto::RecordActivityAtomicallyResponse override {
    return {.ok = true,
            .message = "record: ok\nsync: ok",
            .operation_id = "smoke-txn",
            .warnings = {},
            .rollback_failed = false,
            .retained_transaction_root = std::nullopt};
  }
  auto RunUpdateActivityRemarkAtomically(
      const tracer_core::core::dto::UpdateActivityRemarkAtomicallyRequest&)
      -> tracer_core::core::dto::UpdateActivityRemarkAtomicallyResponse
      override {
    return {.ok = true,
            .message = "remark: ok\nsync: ok",
            .operation_id = "smoke-remark-txn",
            .warnings = {},
            .rollback_failed = false,
            .retained_transaction_root = std::nullopt};
  }
  auto RunUpdateDayRemarkAtomically(
      const tracer_core::core::dto::UpdateDayRemarkAtomicallyRequest&)
      -> tracer_core::core::dto::UpdateDayRemarkAtomicallyResponse override {
    return {.ok = true,
            .message = "day remark: ok\nsync: ok",
            .operation_id = "smoke-day-remark-txn",
            .warnings = {},
            .rollback_failed = false,
            .retained_transaction_root = std::nullopt};
  }
  auto RunDefaultTxtDayMarker(
      const tracer_core::core::dto::DefaultTxtDayMarkerRequest&)
      -> tracer_core::core::dto::DefaultTxtDayMarkerResponse override {
    return {.ok = true, .normalized_day_marker = "0102", .error_message = ""};
  }
  auto RunResolveTxtDayBlock(
      const tracer_core::core::dto::ResolveTxtDayBlockRequest&)
      -> tracer_core::core::dto::ResolveTxtDayBlockResponse override {
    return {.ok = true,
            .normalized_day_marker = "0102",
            .found = true,
            .is_marker_valid = true,
            .can_save = true,
            .day_body = "0904study",
            .day_content_iso_date = std::string("2026-03-29"),
            .error_message = ""};
  }
  auto RunReplaceTxtDayBlock(
      const tracer_core::core::dto::ReplaceTxtDayBlockRequest&)
      -> tracer_core::core::dto::ReplaceTxtDayBlockResponse override {
    return {.ok = true,
            .normalized_day_marker = "0102",
            .found = true,
            .is_marker_valid = true,
            .updated_content = "updated-content\n",
            .error_message = ""};
  }
  auto RunResolveTxtDayEdit(
      const tracer_core::core::dto::ResolveTxtDayEditRequest&)
      -> tracer_core::core::dto::ResolveTxtDayEditResponse override {
    return {.ok = true,
            .normalized_day_marker = "0102",
            .found = true,
            .is_marker_valid = true,
            .can_save = true,
            .day_remark = "",
            .events = {},
            .day_content_iso_date = std::string("2026-03-29"),
            .error_message = ""};
  }
  auto RunApplyTxtDayEdit(
      const tracer_core::core::dto::ApplyTxtDayEditRequest& request)
      -> tracer_core::core::dto::ApplyTxtDayEditResponse override {
    return {.ok = true,
            .normalized_day_marker = "0102",
            .found = true,
            .is_marker_valid = true,
            .updated_content = request.content,
            .error_message = ""};
  }
  auto RunConvertTxtActivityNames(
      const tracer_core::core::dto::ConvertTxtActivityNamesRequest& request)
      -> tracer_core::core::dto::ConvertTxtActivityNamesResponse override {
    return {
        .ok = true, .converted_content = request.content, .error_message = ""};
  }
  auto RunReplaceTxtCanonicalActivityNames(
      const tracer_core::core::dto::ReplaceTxtCanonicalActivityNamesRequest&
          request)
      -> tracer_core::core::dto::ReplaceTxtCanonicalActivityNamesResponse
      override {
    return {
        .ok = true, .updated_content = request.content, .error_message = ""};
  }
  auto RunReplaceTxtAliasActivityNames(
      const tracer_core::core::dto::ReplaceTxtAliasActivityNamesRequest&
          request)
      -> tracer_core::core::dto::ReplaceTxtAliasActivityNamesResponse override {
    return {
        .ok = true, .updated_content = request.content, .error_message = ""};
  }
  auto InstallActiveConverterConfig(const tracer::core::application::pipeline::
                                        ActiveConverterConfigInstallRequest&)
      -> void override {}
};

class SmokeInsightsHandler final : public IInsightsHandler {
 public:
  auto RunDailyQuery(std::string_view, InsightsFormat) -> std::string override {
    return "smoke-daily";
  }
  auto RunMonthlyQuery(std::string_view, InsightsFormat)
      -> std::string override {
    return "smoke-monthly";
  }
  auto RunPeriodQuery(int, InsightsFormat) -> std::string override {
    return "smoke-period";
  }
  auto RunWeeklyQuery(std::string_view, InsightsFormat)
      -> std::string override {
    return "smoke-weekly";
  }
  auto RunYearlyQuery(std::string_view, InsightsFormat)
      -> std::string override {
    return "smoke-yearly";
  }
  auto RunPeriodQueries(const std::vector<int>&, InsightsFormat)
      -> std::string override {
    return "smoke-period-batch";
  }
};

class SmokeDataQueryService final
    : public tracer_core::application::ports::IDataQueryService {
 public:
  auto RunDataQuery(const tracer_core::core::dto::DataQueryRequest&)
      -> tracer_core::core::dto::TextOutput override {
    return {.ok = true, .content = "smoke-data-query", .error_message = ""};
  }
};

class SmokeInsightsDataQueryService final
    : public tracer_core::application::ports::IInsightsDataQueryService {
 public:
  auto QueryDaily(std::string_view date) -> DailyInsightsData override {
    DailyInsightsData insights;
    insights.date = std::string(date);
    return insights;
  }
  auto QueryMonthly(std::string_view month) -> MonthlyInsightsData override {
    MonthlyInsightsData insights;
    insights.range_label = std::string(month);
    return insights;
  }
  auto QueryPeriod(int days) -> PeriodInsightsData override {
    PeriodInsightsData insights;
    insights.requested_days = days;
    return insights;
  }
  auto QueryRange(std::string_view start_date, std::string_view end_date)
      -> PeriodInsightsData override {
    PeriodInsightsData insights;
    insights.start_date = std::string(start_date);
    insights.end_date = std::string(end_date);
    return insights;
  }
  auto QueryWeekly(std::string_view iso_week) -> WeeklyInsightsData override {
    WeeklyInsightsData insights;
    insights.range_label = std::string(iso_week);
    return insights;
  }
  auto QueryYearly(std::string_view year) -> YearlyInsightsData override {
    YearlyInsightsData insights;
    insights.range_label = std::string(year);
    return insights;
  }
  auto ListDailyTargets() -> std::vector<std::string> override { return {}; }
  auto ListMonthlyTargets() -> std::vector<std::string> override { return {}; }
  auto ListWeeklyTargets() -> std::vector<std::string> override { return {}; }
  auto ListYearlyTargets() -> std::vector<std::string> override { return {}; }
  auto QueryPeriodBatch(const std::vector<int>&)
      -> std::map<int, PeriodInsightsData> override {
    return {};
  }
  auto QueryAllDaily() -> std::map<std::string, DailyInsightsData> override {
    return {};
  }
  auto QueryAllMonthly()
      -> std::map<std::string, MonthlyInsightsData> override {
    return {};
  }
  auto QueryAllWeekly() -> std::map<std::string, WeeklyInsightsData> override {
    return {};
  }
  auto QueryAllYearly() -> std::map<std::string, YearlyInsightsData> override {
    return {};
  }
};

class SmokeInsightsFormatter final
    : public tracer_core::application::ports::IInsightsDtoFormatter {
 public:
  auto FormatDaily(const DailyInsightsData& insights, InsightsFormat)
      -> std::string override {
    return "smoke-daily:" + insights.date;
  }
  auto FormatMonthly(const MonthlyInsightsData& insights, InsightsFormat)
      -> std::string override {
    return "smoke-month:" + insights.range_label;
  }
  auto FormatPeriod(const PeriodInsightsData& insights, InsightsFormat)
      -> std::string override {
    return "smoke-period:" + insights.start_date + "|" + insights.end_date;
  }
  auto FormatWeekly(const WeeklyInsightsData& insights, InsightsFormat)
      -> std::string override {
    return "smoke-week:" + insights.range_label;
  }
  auto FormatYearly(const YearlyInsightsData& insights, InsightsFormat)
      -> std::string override {
    return "smoke-year:" + insights.range_label;
  }
};

class SmokeProjectRepository final : public IProjectRepository {
 public:
  auto GetAllProjects() -> std::vector<ProjectEntity> override {
    return {{.id = 1, .parent_id = std::nullopt, .name = "root"}};
  }
};

auto Expect(bool condition, std::string_view message, int& failures) -> void {
  if (condition) {
    return;
  }
  ++failures;
  std::cerr << "[FAIL] " << message << '\n';
}

auto RunAggregateRuntimeSmoke(int& failures) -> void {
  Expect(std::is_class_v<app_use_cases::ITracerCoreRuntime>,
         "ITracerCoreRuntime should be visible through aggregate runtime path.",
         failures);
  Expect(std::is_abstract_v<app_use_cases::ITracerCoreRuntime>,
         "ITracerCoreRuntime should remain abstract.", failures);
  Expect(std::is_base_of_v<app_use_cases::ITracerCoreRuntime,
                           app_use_cases::TracerCoreRuntime>,
         "TracerCoreRuntime should inherit ITracerCoreRuntime.", failures);
  Expect(std::is_class_v<app_use_cases::IPipelineApi>,
         "IPipelineApi should be visible through aggregate runtime path.",
         failures);
  Expect(std::is_class_v<app_use_cases::IQueryApi>,
         "IQueryApi should be visible through aggregate runtime path.",
         failures);
  Expect(std::is_class_v<app_use_cases::IInsightsApi>,
         "IInsightsApi should be visible through aggregate runtime path.",
         failures);
  Expect(std::is_class_v<app_use_cases::ITracerExchangeApi>,
         "ITracerExchangeApi should be visible through aggregate runtime path.",
         failures);

  auto (*build_error_message_fn)(std::string_view, std::string_view)
      ->std::string = &app_use_cases::helpers::BuildErrorMessage;
  auto (*build_text_failure_fn)(std::string_view)
      ->tracer_core::core::dto::TextOutput =
      &app_use_cases::helpers::BuildTextFailure;
  Expect(build_error_message_fn != nullptr,
         "BuildErrorMessage should remain exported.", failures);
  Expect(build_text_failure_fn != nullptr,
         "BuildTextFailure should remain exported.", failures);

  const auto helper_failure =
      app_use_cases::helpers::BuildTextFailure("RunDataQuery", "boom");
  Expect(!helper_failure.ok &&
             helper_failure.error_message == "RunDataQuery failed: boom",
         "BuildTextFailure should preserve aggregate failure wording.",
         failures);

  try {
    SmokePipelineWorkflow pipeline_workflow;
    SmokeInsightsHandler insights_handler;
    auto project_repository = std::make_shared<SmokeProjectRepository>();
    auto data_query_service = std::make_shared<SmokeDataQueryService>();
    auto insights_data_query_service =
        std::make_shared<SmokeInsightsDataQueryService>();
    auto insights_formatter = std::make_shared<SmokeInsightsFormatter>();
    auto pipeline_api =
        std::make_shared<app_use_cases::PipelineApi>(pipeline_workflow);
    auto query_api = std::make_shared<app_use_cases::QueryApi>(
        project_repository, data_query_service);
    auto insights_api = std::make_shared<app_use_cases::InsightsApi>(
        insights_handler, insights_data_query_service, insights_formatter);
    auto tracer_exchange_api =
        std::make_shared<app_use_cases::TracerExchangeApi>();
    app_use_cases::TracerCoreRuntime runtime_api(
        std::move(pipeline_api), std::move(query_api), std::move(insights_api),
        std::move(tracer_exchange_api));

    const auto data_query_result = runtime_api.query().RunDataQuery({});
    Expect(
        data_query_result.ok && data_query_result.content == "smoke-data-query",
        "Aggregate runtime should dispatch query API.", failures);

    const auto insights_query_result =
        runtime_api.insights().RunTemporalInsightsQuery(
            {.display_mode = tracer_core::core::dto::InsightsDisplayMode::kDay,
             .selection =
                 {.kind =
                      tracer_core::core::dto::TemporalSelectionKind::kSingleDay,
                  .date = "2026-03-10"},
             .format = InsightsFormat::kMarkdown});
    Expect(insights_query_result.ok &&
               insights_query_result.content == "smoke-daily:2026-03-10",
           "Aggregate runtime should dispatch insights API.", failures);
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] Aggregate runtime smoke should construct and execute: "
              << exception.what() << '\n';
  } catch (...) {
    ++failures;
    std::cerr << "[FAIL] Aggregate runtime smoke should construct and execute: "
              << "unknown non-standard exception\n";
  }
}

}  // namespace

auto main() -> int {
  int failures = 0;
  RunAggregateRuntimeSmoke(failures);
  if (failures == 0) {
    std::cout
        << "[PASS] tracer_core_application_aggregate_runtime_smoke_tests\n";
    return 0;
  }
  std::cerr << "[FAIL] tracer_core_application_aggregate_runtime_smoke_tests "
               "failures: "
            << failures << '\n';
  return 1;
}
