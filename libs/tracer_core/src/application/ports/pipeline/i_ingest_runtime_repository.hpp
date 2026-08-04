#ifndef APPLICATION_PORTS_I_INGEST_RUNTIME_REPOSITORY_H_
#define APPLICATION_PORTS_I_INGEST_RUNTIME_REPOSITORY_H_

#include <optional>
#include <string>
#include <string_view>

#include "application/dto/pipeline_responses.hpp"

namespace tracer_core::application::ports {

struct PreviousActivityTail {
  std::string date;
  std::string end_time;
};

// Read-side persistence boundary used by ingest synchronization and timeline
// continuity checks. It must never create a missing database.
class IIngestRuntimeRepository {
 public:
  virtual ~IIngestRuntimeRepository() = default;

  [[nodiscard]] virtual auto ListIngestSyncStatuses(
      const tracer_core::core::dto::IngestSyncStatusRequest& request) const
      -> tracer_core::core::dto::IngestSyncStatusOutput = 0;
  [[nodiscard]] virtual auto TryGetLatestActivityTailBeforeDate(
      std::string_view date) const -> std::optional<PreviousActivityTail> = 0;
};

}  // namespace tracer_core::application::ports

#endif  // APPLICATION_PORTS_I_INGEST_RUNTIME_REPOSITORY_H_
