#ifndef DOMAIN_INSIGHTS_MODELS_ACTIVITY_AGGREGATE_H_
#define DOMAIN_INSIGHTS_MODELS_ACTIVITY_AGGREGATE_H_

#include <cstdint>

struct ActivityAggregate {
  std::int64_t total_duration_seconds = 0;
  std::int64_t occurrence_count = 0;

  void AddDuration(std::int64_t duration_seconds) {
    total_duration_seconds += duration_seconds;
  }

  void AddOccurrences(std::int64_t occurrences) {
    occurrence_count += occurrences;
  }

  void Add(std::int64_t duration_seconds, std::int64_t occurrences = 1) {
    AddDuration(duration_seconds);
    AddOccurrences(occurrences);
  }

  [[nodiscard]] auto AverageDurationPerOccurrenceSeconds() const
      -> std::int64_t {
    return occurrence_count <= 0 ? 0
                                 : total_duration_seconds / occurrence_count;
  }
};

#endif  // DOMAIN_INSIGHTS_MODELS_ACTIVITY_AGGREGATE_H_
