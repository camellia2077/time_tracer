#ifndef INFRASTRUCTURE_PERSISTENCE_REPOSITORIES_SQLITE_INGEST_RUNTIME_REPOSITORY_H_
#define INFRASTRUCTURE_PERSISTENCE_REPOSITORIES_SQLITE_INGEST_RUNTIME_REPOSITORY_H_

#include <string>

#include "application/ports/pipeline/i_ingest_runtime_repository.hpp"

namespace tracer::core::infrastructure::persistence {

class SqliteIngestRuntimeRepository final
    : public tracer_core::application::ports::IIngestRuntimeRepository {
 public:
  explicit SqliteIngestRuntimeRepository(std::string db_path);

  [[nodiscard]] auto ListIngestSyncStatuses(
      const tracer_core::core::dto::IngestSyncStatusRequest& request) const
      -> tracer_core::core::dto::IngestSyncStatusOutput override;
  [[nodiscard]] auto TryGetLatestActivityTailBeforeDate(std::string_view date)
      const -> std::optional<
          tracer_core::application::ports::PreviousActivityTail> override;

 private:
  std::string db_path_;
};

}  // namespace tracer::core::infrastructure::persistence

#endif  // INFRASTRUCTURE_PERSISTENCE_REPOSITORIES_SQLITE_INGEST_RUNTIME_REPOSITORY_H_
