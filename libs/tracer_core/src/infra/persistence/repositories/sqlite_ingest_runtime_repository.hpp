#ifndef INFRASTRUCTURE_PERSISTENCE_REPOSITORIES_SQLITE_INGEST_RUNTIME_REPOSITORY_H_
#define INFRASTRUCTURE_PERSISTENCE_REPOSITORIES_SQLITE_INGEST_RUNTIME_REPOSITORY_H_

#include <string>

#include "application/ports/pipeline/i_ingest_runtime_repository.hpp"
#include "application/ports/query/i_runtime_activity_query_repository.hpp"

namespace tracer::core::infrastructure::persistence {

class SqliteIngestRuntimeRepository final
    : public tracer_core::application::ports::IIngestRuntimeRepository,
      public tracer_core::application::ports::IRuntimeActivityQueryRepository {
 public:
  explicit SqliteIngestRuntimeRepository(std::string db_path);

  [[nodiscard]] auto ListIngestSyncStatuses(
      const tracer_core::core::dto::IngestSyncStatusRequest& request) const
      -> tracer_core::core::dto::IngestSyncStatusOutput override;
  [[nodiscard]] auto TryGetLatestActivityTailBeforeDate(std::string_view date)
      const -> std::optional<
          tracer_core::application::ports::PreviousActivityTail> override;
  [[nodiscard]] auto TryGetLatestActivityTailAtOrBeforeDate(
      std::string_view date) const -> std::optional<
          tracer_core::application::ports::ActivityTailQueryResult> override;
  [[nodiscard]] auto TryGetLatestActivityRecordOnDate(std::string_view date)
      const -> std::optional<
          tracer_core::application::ports::LatestActivityRecord> override;

 private:
  std::string db_path_;
};

}  // namespace tracer::core::infrastructure::persistence

#endif  // INFRASTRUCTURE_PERSISTENCE_REPOSITORIES_SQLITE_INGEST_RUNTIME_REPOSITORY_H_
