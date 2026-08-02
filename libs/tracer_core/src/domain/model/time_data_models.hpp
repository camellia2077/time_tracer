// domain/model/time_data_models.hpp
#ifndef DOMAIN_MODEL_TIME_DATA_MODELS_H_
#define DOMAIN_MODEL_TIME_DATA_MODELS_H_

#include <optional>
#include <string>
#include <utility>

#include "domain/model/source_span.hpp"

// 统一的统计结构 (对应原 GeneratedStats，但字段名为 snake_case)
struct ActivityStats {
  int sleep_night_time = 0;
  int sleep_day_time = 0;
  int sleep_total_time = 0;

  int total_exercise_time = 0;
  int cardio_time = 0;
  int anaerobic_time = 0;

  int grooming_time = 0;
  int toilet_time = 0;
  int gaming_time = 0;

  int recreation_time = 0;
  int recreation_zhihu_time = 0;
  int recreation_bilibili_time = 0;
  int recreation_douyin_time = 0;

  int study_time = 0;
};

// A canonical activity record may be a resolved interval or an authored point
// whose start boundary is not currently knowable. The latter is still a
// reportable activity fact, but it must not contribute to duration totals.
enum class ActivityRecordKind {
  kInterval,
  kEndOnly,
};

[[nodiscard]] constexpr auto IsEndOnly(ActivityRecordKind kind) noexcept
    -> bool {
  return kind == ActivityRecordKind::kEndOnly;
}

// 统一的基础活动记录 (对应原 Activity / TimeRecordInternal)
struct BaseActivityRecord {
  [[nodiscard]] static auto MakeInterval(std::string start_time,
                                          std::string end_time,
                                          std::string project)
      -> BaseActivityRecord {
    BaseActivityRecord record;
    record.kind = ActivityRecordKind::kInterval;
    record.start_time_str = std::move(start_time);
    record.end_time_str = std::move(end_time);
    record.project_path = std::move(project);
    return record;
  }

  [[nodiscard]] static auto MakeEndOnly(std::string end_time,
                                         std::string project)
      -> BaseActivityRecord {
    BaseActivityRecord record;
    record.kind = ActivityRecordKind::kEndOnly;
    record.end_time_str = std::move(end_time);
    record.project_path = std::move(project);
    return record;
  }

  [[nodiscard]] auto IsEndOnly() const noexcept -> bool {
    return ::IsEndOnly(kind);
  }

  // This is the boundary invariant before timestamps and duration are
  // derived: end-only activities have no start boundary; intervals have both.
  [[nodiscard]] auto HasValidBoundaryShape() const noexcept -> bool {
    if (IsEndOnly()) {
      return start_time_str.empty() && !end_time_str.empty();
    }
    return !start_time_str.empty() && !end_time_str.empty();
  }

  ActivityRecordKind kind = ActivityRecordKind::kInterval;
  long long logical_id = 0;
  long long start_timestamp = 0;
  long long end_timestamp = 0;

  std::string start_time_str;  // 原 startTime / start
  std::string end_time_str;    // 原 endTime / end
  std::string project_path;

  int duration_seconds = 0;           // 原 durationSeconds
  std::optional<std::string> remark;  // 原 activityRemark
  std::optional<SourceSpan> source_span;
};

namespace tracer::core::domain::model {

#include "domain/detail/time_data_models_contract.inc"

}  // namespace tracer::core::domain::model

#endif  // DOMAIN_MODEL_TIME_DATA_MODELS_H_
