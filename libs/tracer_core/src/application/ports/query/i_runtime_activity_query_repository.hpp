#ifndef APPLICATION_PORTS_QUERY_I_RUNTIME_ACTIVITY_QUERY_REPOSITORY_H_
#define APPLICATION_PORTS_QUERY_I_RUNTIME_ACTIVITY_QUERY_REPOSITORY_H_

#include <optional>
#include <string>
#include <string_view>

namespace tracer_core::application::ports {

struct ActivityTailQueryResult {
  std::string date;
  std::string end_time;
};

struct LatestActivityRecord {
  std::string date;
  std::string activity;
  std::string record_kind;
  std::string start_time;
  std::string end_time;
  int duration_seconds = 0;
};

// Read-only persisted activity queries owned by the query capability.
// Implementations must never create a missing database.
class IRuntimeActivityQueryRepository {
 public:
  virtual ~IRuntimeActivityQueryRepository() = default;

  [[nodiscard]] virtual auto TryGetLatestActivityTailAtOrBeforeDate(
      std::string_view date) const
      -> std::optional<ActivityTailQueryResult> = 0;
  [[nodiscard]] virtual auto TryGetLatestActivityRecordOnDate(
      std::string_view date) const
      -> std::optional<LatestActivityRecord> = 0;
};

}  // namespace tracer_core::application::ports

#endif  // APPLICATION_PORTS_QUERY_I_RUNTIME_ACTIVITY_QUERY_REPOSITORY_H_
