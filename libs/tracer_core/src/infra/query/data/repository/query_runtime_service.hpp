// infra/query/data/repository/query_runtime_service.hpp
#ifndef INFRASTRUCTURE_QUERY_DATA_REPOSITORY_QUERY_RUNTIME_SERVICE_H_
#define INFRASTRUCTURE_QUERY_DATA_REPOSITORY_QUERY_RUNTIME_SERVICE_H_

#include <filesystem>
#include <optional>

#include "application/ports/query/i_data_query_service.hpp"

namespace tracer::core::infrastructure::query::data::repository {

class QueryRuntimeService final
    : public tracer_core::application::ports::IDataQueryService {
 public:
  explicit QueryRuntimeService(
      std::filesystem::path db_path,
      std::optional<std::filesystem::path> converter_config_toml_path =
          std::nullopt);

  auto RunDataQuery(const tracer_core::core::dto::DataQueryRequest& request)
      -> tracer_core::core::dto::TextOutput override;

 private:
  std::filesystem::path db_path_;
  std::optional<std::filesystem::path> converter_config_toml_path_;
};

}  // namespace tracer::core::infrastructure::query::data::repository

namespace tracer_core::infrastructure::query::data::repository {

using tracer::core::infrastructure::query::data::repository::QueryRuntimeService;

}  // namespace tracer_core::infrastructure::query::data::repository

#endif  // INFRASTRUCTURE_QUERY_DATA_REPOSITORY_QUERY_RUNTIME_SERVICE_H_
