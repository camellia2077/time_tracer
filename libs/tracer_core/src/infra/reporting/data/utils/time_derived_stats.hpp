#ifndef INFRASTRUCTURE_REPORTS_DATA_UTILS_TIME_DERIVED_STATS_H_
#define INFRASTRUCTURE_REPORTS_DATA_UTILS_TIME_DERIVED_STATS_H_

#include <cstdint>
#include <map>
#include <string>
#include <string_view>

#include "domain/model/time_data_models.hpp"
#include "infra/schema/day_schema.hpp"

namespace tracer::core::infrastructure::reports::data::stats {

namespace detail {

[[nodiscard]] inline auto PathEqualsOrPrefix(std::string_view path,
                                             std::string_view prefix) -> bool {
  return (path == prefix) ||
         (path.size() > prefix.size() && path.starts_with(prefix) &&
          path[prefix.size()] == '_');
}

[[nodiscard]] inline auto PathHasRoot(std::string_view path,
                                      std::string_view root) -> bool {
  return PathEqualsOrPrefix(path, root);
}

[[nodiscard]] inline auto PathLeaf(std::string_view path) -> std::string_view {
  const size_t separator = path.rfind('_');
  if (separator == std::string_view::npos) {
    return path;
  }
  return path.substr(separator + 1);
}

[[nodiscard]] inline auto PathHasSegment(std::string_view path,
                                         std::string_view segment) -> bool {
  size_t start = 0;
  while (start <= path.size()) {
    const size_t separator = path.find('_', start);
    const std::string_view part = path.substr(
        start, separator == std::string_view::npos ? std::string_view::npos
                                                   : separator - start);
    if (part == segment) {
      return true;
    }
    if (separator == std::string_view::npos) {
      return false;
    }
    start = separator + 1;
  }
  return false;
}

}  // namespace detail

[[nodiscard]] inline auto IsCardioProjectPath(std::string_view project_path)
    -> bool {
  return detail::PathHasRoot(project_path, "exercise_cardio");
}

[[nodiscard]] inline auto IsAnaerobicProjectPath(std::string_view project_path)
    -> bool {
  return detail::PathHasRoot(project_path, "exercise_anaerobic");
}

[[nodiscard]] inline auto IsStudyProjectPath(std::string_view project_path)
    -> bool {
  return detail::PathHasRoot(project_path, "study");
}

[[nodiscard]] inline auto IsExerciseProjectPath(std::string_view project_path)
    -> bool {
  return detail::PathHasRoot(project_path, "exercise");
}

class DerivedTimeStatsAggregator {
 public:
  auto AddPathDuration(std::string_view project_path,
                       std::int64_t duration_seconds) -> void {
    if (project_path.empty() || duration_seconds <= 0) {
      return;
    }

    if (project_path == "sleep_night") {
      stats_.sleep_night_time += static_cast<int>(duration_seconds);
    }
    if (project_path == "sleep_day") {
      stats_.sleep_day_time += static_cast<int>(duration_seconds);
    }

    if (IsStudyProjectPath(project_path)) {
      stats_.study_time += static_cast<int>(duration_seconds);
    }

    if (IsExerciseProjectPath(project_path)) {
      stats_.total_exercise_time += static_cast<int>(duration_seconds);
    }
    if (IsCardioProjectPath(project_path)) {
      stats_.cardio_time += static_cast<int>(duration_seconds);
    }
    if (IsAnaerobicProjectPath(project_path)) {
      stats_.anaerobic_time += static_cast<int>(duration_seconds);
    }

    if (detail::PathHasRoot(project_path, "recreation")) {
      stats_.recreation_time += static_cast<int>(duration_seconds);
      const std::string_view kLeaf = detail::PathLeaf(project_path);
      if (kLeaf == "zhihu") {
        stats_.recreation_zhihu_time += static_cast<int>(duration_seconds);
      } else if (kLeaf == "bilibili") {
        stats_.recreation_bilibili_time += static_cast<int>(duration_seconds);
      } else if (kLeaf == "douyin") {
        stats_.recreation_douyin_time += static_cast<int>(duration_seconds);
      }
    }
    if (detail::PathHasRoot(project_path, "recreation_game")) {
      stats_.gaming_time += static_cast<int>(duration_seconds);
    }

    if (detail::PathHasRoot(project_path, "routine_toilet")) {
      stats_.toilet_time += static_cast<int>(duration_seconds);
    }
    if (detail::PathHasRoot(project_path, "routine") &&
        (detail::PathHasSegment(project_path, "grooming") ||
         detail::PathHasSegment(project_path, "body-hygiene") ||
         detail::PathHasSegment(project_path, "personal-hygiene") ||
         detail::PathHasSegment(project_path, "oral-hygiene"))) {
      stats_.grooming_time += static_cast<int>(duration_seconds);
    }
  }

  [[nodiscard]] auto BuildActivityStats() const -> ActivityStats {
    ActivityStats result = stats_;
    result.sleep_total_time = result.sleep_night_time + result.sleep_day_time;
    return result;
  }

  [[nodiscard]] auto BuildReportStatsMap() const
      -> std::map<std::string, std::int64_t> {
    const ActivityStats result = BuildActivityStats();
    return {
        {std::string(schema::day::stats::kSleepNightTime),
         result.sleep_night_time},
        {std::string(schema::day::stats::kSleepDayTime), result.sleep_day_time},
        {std::string(schema::day::stats::kSleepTotalTime),
         result.sleep_total_time},
        {std::string(schema::day::stats::kTotalExerciseTime),
         result.total_exercise_time},
        {std::string(schema::day::stats::kCardioTime), result.cardio_time},
        {std::string(schema::day::stats::kAnaerobicTime),
         result.anaerobic_time},
        {std::string(schema::day::stats::kGroomingTime), result.grooming_time},
        {std::string(schema::day::stats::kToiletTime), result.toilet_time},
        {std::string(schema::day::stats::kGamingTime), result.gaming_time},
        {std::string(schema::day::stats::kRecreationTime),
         result.recreation_time},
        {std::string(schema::day::stats::kRecreationZhihuTime),
         result.recreation_zhihu_time},
        {std::string(schema::day::stats::kRecreationBilibiliTime),
         result.recreation_bilibili_time},
        {std::string(schema::day::stats::kRecreationDouyinTime),
         result.recreation_douyin_time},
        {std::string(schema::day::stats::kStudyTime), result.study_time},
    };
  }

  [[nodiscard]] auto HasCardioActivity() const -> bool {
    return stats_.cardio_time > 0;
  }

  [[nodiscard]] auto HasAnaerobicActivity() const -> bool {
    return stats_.anaerobic_time > 0;
  }

  [[nodiscard]] auto HasStudyActivity() const -> bool {
    return stats_.study_time > 0;
  }

  [[nodiscard]] auto HasExerciseActivity() const -> bool {
    return stats_.total_exercise_time > 0;
  }

 private:
  ActivityStats stats_{};
};

}  // namespace tracer::core::infrastructure::reports::data::stats

#endif  // INFRASTRUCTURE_REPORTS_DATA_UTILS_TIME_DERIVED_STATS_H_
