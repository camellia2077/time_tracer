// infrastructure/persistence/sqlite_data_query_service.hpp
#ifndef INFRASTRUCTURE_PERSISTENCE_SQLITE_DATA_QUERY_SERVICE_H_
#define INFRASTRUCTURE_PERSISTENCE_SQLITE_DATA_QUERY_SERVICE_H_

#include <filesystem>
#include <optional>

#include "application/ports/i_data_query_service.hpp"

namespace infrastructure::persistence {

class SqliteDataQueryService final
    : public tracer_core::application::ports::IDataQueryService {
 public:
  explicit SqliteDataQueryService(
      std::filesystem::path db_path,
      std::optional<std::filesystem::path> converter_config_toml_path =
          std::nullopt);

  auto RunDataQuery(const tracer_core::core::dto::DataQueryRequest& request)
      -> tracer_core::core::dto::TextOutput override;

 private:
  std::filesystem::path db_path_;
  std::optional<std::filesystem::path> converter_config_toml_path_;
};

}  // namespace infrastructure::persistence

#endif  // INFRASTRUCTURE_PERSISTENCE_SQLITE_DATA_QUERY_SERVICE_H_
