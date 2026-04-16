// infra/query/data/repository/query_runtime_service.cpp
#include "infra/query/data/repository/query_runtime_service.hpp"

#include <optional>
#include <string>
#include <utility>

#include "infra/persistence/sqlite/db_manager.hpp"
#include "infra/query/data/repository/query_runtime_service_internal.hpp"

import tracer.core.infrastructure.query.data.renderers;

namespace tracer::core::infrastructure::query::data::repository {
namespace infra_data_query_renderers =
    tracer::core::infrastructure::query::data::renderers;
namespace runtime_service_internal =
    tracer::core::infrastructure::query::data::repository::internal;

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

  if (request.action == tracer_core::core::dto::DataQueryAction::kReportChart) {
    runtime_service_internal::ValidateReportChartRequest(request);
  }
  if (request.action ==
      tracer_core::core::dto::DataQueryAction::kReportComposition) {
    runtime_service_internal::ValidateReportCompositionRequest(request);
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
