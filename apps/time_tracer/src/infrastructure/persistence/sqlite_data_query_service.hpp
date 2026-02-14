// infrastructure/persistence/sqlite_data_query_service.hpp
#ifndef INFRASTRUCTURE_PERSISTENCE_SQLITE_DATA_QUERY_SERVICE_H_
#define INFRASTRUCTURE_PERSISTENCE_SQLITE_DATA_QUERY_SERVICE_H_

#include <filesystem>

#include "application/ports/i_data_query_service.hpp"

namespace infrastructure::persistence {

class SqliteDataQueryService final
    : public time_tracer::application::ports::IDataQueryService {
 public:
  explicit SqliteDataQueryService(std::filesystem::path db_path);

  auto RunDataQuery(const time_tracer::core::dto::DataQueryRequest& request)
      -> time_tracer::core::dto::TextOutput override;

 private:
  std::filesystem::path db_path_;
};

}  // namespace infrastructure::persistence

#endif  // INFRASTRUCTURE_PERSISTENCE_SQLITE_DATA_QUERY_SERVICE_H_
