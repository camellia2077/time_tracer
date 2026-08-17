// domain/logic/validator/structure/structure_validator.hpp
#ifndef DOMAIN_LOGIC_VALIDATOR_STRUCTURE_STRUCTURE_VALIDATOR_H_
#define DOMAIN_LOGIC_VALIDATOR_STRUCTURE_STRUCTURE_VALIDATOR_H_

#include <cstddef>
#include <optional>
#include <unordered_set>
#include <string>
#include <vector>

#include "domain/logic/validator/common/diagnostic.hpp"
#include "domain/model/daily_log.hpp"
#include "domain/types/date_check_mode.hpp"

namespace validator::structure {

enum class MixedTimelineIssueCode {
  kMissingIntervalStart,
  kInvalidIntervalRange,
  kOverlap,
  kWakeIntervalNotAllowed,
};

struct MixedTimelineIssue {
  std::size_t event_index = 0;
  MixedTimelineIssueCode code = MixedTimelineIssueCode::kOverlap;
};

// Timeline coordinates are elapsed seconds from the authored day's first
// midnight. They may exceed 24 hours when a valid event sequence crosses
// midnight. Point events have equal start/end coordinates.
struct MixedTimelineEventBounds {
  bool participates_in_timeline = false;
  std::optional<int> start_timeline_seconds;
  std::optional<int> end_timeline_seconds;
  std::optional<int> previous_end_timeline_seconds;
  std::optional<int> next_start_timeline_seconds;
};

struct MixedTimelineAnalysis {
  std::vector<MixedTimelineEventBounds> event_bounds;
  std::vector<MixedTimelineIssue> issues;

  [[nodiscard]] auto ok() const -> bool { return issues.empty(); }
};

// Shared parser-to-timeline projection used by structural validation and
// interactive day editing. It keeps the established mixed point/interval and
// cross-midnight semantics in one Core implementation.
[[nodiscard]] auto AnalyzeMixedTimeline(
    const DailyLog& day, const std::unordered_set<std::string>& wake_keywords)
    -> MixedTimelineAnalysis;

class StructValidator {
 public:
  explicit StructValidator(DateCheckMode mode = DateCheckMode::kNone,
                           std::vector<std::string> wake_keywords = {});

  auto Validate(const std::string& filename, const std::vector<DailyLog>& days,
                std::vector<Diagnostic>& diagnostics) -> bool;

 private:
  DateCheckMode date_check_mode_;
  std::unordered_set<std::string> wake_keywords_;
};

}  // namespace validator::structure

#endif  // DOMAIN_LOGIC_VALIDATOR_STRUCTURE_STRUCTURE_VALIDATOR_H_
