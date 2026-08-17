// infra/query/data/repository/query_runtime_service.cpp
#include "infra/query/data/repository/query_runtime_service.hpp"

#include <nlohmann/json.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "application/ports/query/i_runtime_activity_query_repository.hpp"
#include "infra/persistence/sqlite/db_manager.hpp"
#include "infra/query/data/repository/query_runtime_service_internal.hpp"

import tracer.core.infrastructure.query.data.renderers;
import tracer.core.infrastructure.persistence.runtime;

namespace tracer::core::infrastructure::query::data::repository {
namespace infra_data_query_renderers =
    tracer::core::infrastructure::query::data::renderers;
namespace runtime_service_internal =
    tracer::core::infrastructure::query::data::repository::internal;
namespace infra_persistence = tracer::core::infrastructure::persistence;
using nlohmann::json;
using tracer_core::application::ports::IRuntimeActivityQueryRepository;
using tracer_core::core::dto::TextOutput;

auto BuildRuntimeQuerySuccess(
    std::string_view action, json payload,
    const tracer_core::core::dto::DataQueryOutputMode kOutputMode)
    -> TextOutput {
  std::string content = infra_data_query_renderers::RenderJsonObjectOutput(
      action, payload.dump(), kOutputMode);
  return {.ok = true, .content = std::move(content), .error_message = ""};
}

auto RunPreviousActivityTailQuery(
    const IRuntimeActivityQueryRepository& repository, std::string_view date,
    const tracer_core::core::dto::DataQueryOutputMode kOutputMode)
    -> TextOutput {
  const auto kTail = repository.TryGetLatestActivityTailAtOrBeforeDate(date);
  json payload = {
      {"found", kTail.has_value()},
  };
  if (kTail.has_value()) {
    payload["date"] = kTail->date;
    payload["end_time"] = kTail->end_time;
  }
  return BuildRuntimeQuerySuccess("previous_activity_tail", std::move(payload),
                                  kOutputMode);
}

auto RunLatestActivityRecordQuery(
    const IRuntimeActivityQueryRepository& repository, std::string_view date,
    const tracer_core::core::dto::DataQueryOutputMode kOutputMode)
    -> TextOutput {
  const auto kRecord = repository.TryGetLatestActivityRecordOnDate(date);
  json payload = {
      {"found", kRecord.has_value()},
  };
  if (kRecord.has_value()) {
    payload["date"] = kRecord->date;
    payload["activity"] = kRecord->activity;
    payload["record_kind"] = kRecord->record_kind;
    payload["start_time"] = kRecord->start_time;
    payload["end_time"] = kRecord->end_time;
    payload["duration_seconds"] = kRecord->duration_seconds;
  }
  return BuildRuntimeQuerySuccess("latest_activity_record", std::move(payload),
                                  kOutputMode);
}

QueryRuntimeService::QueryRuntimeService(
    std::filesystem::path db_path,
    std::optional<std::filesystem::path> converter_config_toml_path)
    : db_path_(std::move(db_path)),
      converter_config_toml_path_(std::move(converter_config_toml_path)) {}

auto QueryRuntimeService::RunDataQuery(
    const tracer_core::core::dto::DataQueryRequest& request)
    -> tracer_core::core::dto::TextOutput {
  if (request.action ==
      tracer_core::core::dto::DataQueryAction::kMappingNames) {
    std::string content = runtime_service_internal::BuildMappingNamesContent(
        converter_config_toml_path_);
    content = infra_data_query_renderers::RenderJsonObjectOutput(
        "mapping_names", std::move(content), request.output_mode);
    return {.ok = true, .content = std::move(content), .error_message = ""};
  }
  if (request.action ==
      tracer_core::core::dto::DataQueryAction::kActivityHierarchyLeafMappings) {
    std::string content =
        runtime_service_internal::BuildActivityHierarchyLeafMappingsContent(
            converter_config_toml_path_);
    content = infra_data_query_renderers::RenderJsonObjectOutput(
        "activity_alias_mappings", std::move(content), request.output_mode);
    return {.ok = true, .content = std::move(content), .error_message = ""};
  }
  if (request.action ==
      tracer_core::core::dto::DataQueryAction::kMappingAliasKeys) {
    std::string content =
        runtime_service_internal::BuildMappingAliasKeysContent(
            converter_config_toml_path_);
    content = infra_data_query_renderers::RenderJsonObjectOutput(
        "mapping_alias_keys", std::move(content), request.output_mode);
    return {.ok = true, .content = std::move(content), .error_message = ""};
  }
  if (request.action ==
      tracer_core::core::dto::DataQueryAction::kWakeKeywords) {
    std::string content = runtime_service_internal::BuildWakeKeywordsContent(
        converter_config_toml_path_);
    content = infra_data_query_renderers::RenderJsonObjectOutput(
        "wake_keywords", std::move(content), request.output_mode);
    return {.ok = true, .content = std::move(content), .error_message = ""};
  }
  if (request.action ==
      tracer_core::core::dto::DataQueryAction::kAuthorableEventTokens) {
    std::string content =
        runtime_service_internal::BuildAuthorableEventTokensContent(
            converter_config_toml_path_);
    content = infra_data_query_renderers::RenderJsonObjectOutput(
        "authorable_event_tokens", std::move(content), request.output_mode);
    return {.ok = true, .content = std::move(content), .error_message = ""};
  }

  if (request.action ==
      tracer_core::core::dto::DataQueryAction::kPreviousActivityTail) {
    if (!request.from_date.has_value() || request.from_date->empty()) {
      return {
          .ok = false,
          .content = "",
          .error_message = "previous_activity_tail query requires from_date."};
    }

    const infra_persistence::SqliteIngestRuntimeRepository kRepository(
        db_path_.string());
    return RunPreviousActivityTailQuery(kRepository, *request.from_date,
                                        request.output_mode);
  }

  if (request.action ==
      tracer_core::core::dto::DataQueryAction::kLatestActivityRecord) {
    if (!request.from_date.has_value() || request.from_date->empty()) {
      return {
          .ok = false,
          .content = "",
          .error_message = "latest_activity_record query requires from_date."};
    }

    const infra_persistence::SqliteIngestRuntimeRepository kRepository(
        db_path_.string());
    return RunLatestActivityRecordQuery(kRepository, *request.from_date,
                                        request.output_mode);
  }

  if (request.action ==
      tracer_core::core::dto::DataQueryAction::kInsightsChart) {
    runtime_service_internal::ValidateInsightsChartRequest(request);
  }
  if (request.action ==
      tracer_core::core::dto::DataQueryAction::kInsightsComposition) {
    runtime_service_internal::ValidateInsightsCompositionRequest(request);
  }

  DBManager db_manager(db_path_.string());
  sqlite3* db_conn =
      runtime_service_internal::EnsureDbConnectionOrThrow(db_manager, db_path_);

  const auto kAction =
      runtime_service_internal::ToCliDataQueryAction(request.action);
  const auto kBaseFilters = runtime_service_internal::BuildCliFilters(request);
  return runtime_service_internal::DispatchDataQueryAction(
      db_conn, request, kAction, kBaseFilters);
}

}  // namespace tracer::core::infrastructure::query::data::repository
