// domain/logic/validator/structure/rules/activity_rules.cpp
#include "domain/logic/validator/structure/rules/activity_rules.hpp"

namespace validator::structure {

void validateActivityCount(const DailyLog& day,
                           std::vector<Diagnostic>& diagnostics) {
  // 检查 activities 数量
  // 规则：每天至少应该有 2 个活动（例如 Sleep 和其他活动），
  // 否则可能会导致某些统计计算异常。
  // 注意：这里使用的是 processedActivities，它是解析和合并后的活动列表
  if (day.processedActivities.size() < 2) {
    diagnostics.push_back(
        {.severity = DiagnosticSeverity::kError,
         .code = "activity.count.too_few",
         .message = "In file for date " + day.date +
                    ": The day has less than 2 activities. This may cause "
                    "issues with 'sleep' activity generation.",
         .source_span = day.source_span});
  }
}

void validateActivityDuration(const DailyLog& day,
                              std::vector<Diagnostic>& diagnostics) {
  constexpr int kSecondsPerHour = 60 * 60;
  constexpr int kMaxActivityDurationSeconds = 16 * kSecondsPerHour;
  constexpr std::string_view kAllowLongToken = "@allow-long";

  for (const auto& activity : day.processedActivities) {
    const int duration_minutes = activity.duration_seconds > 0
                                     ? (activity.duration_seconds / 60)
                                     : 0;
    const int duration_hours = duration_minutes / 60;
    const int duration_remaining_minutes = duration_minutes % 60;
    const std::string duration_text = std::to_string(duration_hours) + "h " +
                                      std::to_string(duration_remaining_minutes) +
                                      "m";

    if (activity.duration_seconds <= 0) {
      const std::string start_time =
          activity.start_time_str.empty() ? "N/A" : activity.start_time_str;
      const std::string end_time =
          activity.end_time_str.empty() ? "N/A" : activity.end_time_str;
      const bool same_time =
          !activity.start_time_str.empty() &&
          (activity.start_time_str == activity.end_time_str);
      const std::string extra_hint =
          same_time ? " (start_time equals end_time)" : "";
      diagnostics.push_back(
          {.severity = DiagnosticSeverity::kError,
           .code = "activity.duration.zero",
           .message =
               "In file for date " + day.date +
               ": Activity duration must be positive (start=" + start_time +
               ", end=" + end_time + ", duration=" + duration_text +
               ", project=" + activity.project_path + ")" + extra_hint + ".",
           .source_span = activity.source_span});
      continue;
    }

    const bool allow_long =
        activity.remark.has_value() &&
        (activity.remark.value().find(kAllowLongToken) != std::string::npos);
    if (!allow_long &&
        activity.duration_seconds > kMaxActivityDurationSeconds) {
      const std::string start_time =
          activity.start_time_str.empty() ? "N/A" : activity.start_time_str;
      const std::string end_time =
          activity.end_time_str.empty() ? "N/A" : activity.end_time_str;
      diagnostics.push_back(
          {.severity = DiagnosticSeverity::kError,
           .code = "activity.duration.too_long",
           .message =
               "In file for date " + day.date +
               ": Activity duration exceeds 16 hours (start=" + start_time +
               ", end=" + end_time + ", duration=" + duration_text +
               ", project=" + activity.project_path +
               "). Use @allow-long in remark to bypass.",
           .source_span = activity.source_span});
    }
  }
}

}  // namespace validator::structure
