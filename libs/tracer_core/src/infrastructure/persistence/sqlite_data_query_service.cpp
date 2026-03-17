// infrastructure/persistence/sqlite_data_query_service.cpp
import tracer.core.infrastructure.query.data.renderers;

#include "infrastructure/persistence/sqlite_data_query_service.hpp"

#include <optional>
#include <string>
#include <utility>

#include "infrastructure/persistence/sqlite/db_manager.hpp"
#include "infrastructure/persistence/sqlite_data_query_service_internal.hpp"

namespace infrastructure::persistence {
namespace infra_data_query_renderers =
    tracer::core::infrastructure::query::data::renderers;

SqliteDataQueryService::SqliteDataQueryService(
    std::filesystem::path db_path,
    std::optional<std::filesystem::path> converter_config_toml_path)
    : db_path_(std::move(db_path)),
      converter_config_toml_path_(std::move(converter_config_toml_path)) {}

auto SqliteDataQueryService::RunDataQuery(
    const tracer_core::core::dto::DataQueryRequest& request)
    -> tracer_core::core::dto::TextOutput {
  if (request.action ==
      tracer_core::core::dto::DataQueryAction::kMappingNames) {
    std::string content =
        infrastructure::persistence::data_query_service_internal::
            BuildMappingNamesContent(converter_config_toml_path_);
    content = infra_data_query_renderers::RenderJsonObjectOutput(
        "mapping_names", std::move(content), request.output_mode);
    return {.ok = true, .content = std::move(content), .error_message = ""};
  }

  if (request.action ==
      tracer_core::core::dto::DataQueryAction::kReportChart) {
    infrastructure::persistence::data_query_service_internal::
        ValidateReportChartRequest(request);
  }

  DBManager db_manager(db_path_.string());
  sqlite3* db_conn = data_query_service_internal::EnsureDbConnectionOrThrow(
      db_manager, db_path_);

  const auto kAction =
      data_query_service_internal::ToCliDataQueryAction(request.action);
  const auto kBaseFilters =
      data_query_service_internal::BuildCliFilters(request);
  return infrastructure::persistence::data_query_service_internal::
      DispatchDataQueryAction(db_conn, request, kAction, kBaseFilters);
}

}  // namespace infrastructure::persistence
