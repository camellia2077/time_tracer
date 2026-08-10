#include "application/use_cases/insights_query_support.hpp"

#include <chrono>
#include <sstream>
#include <stdexcept>

#include "application/use_cases/core_api_failure.hpp"
#include "domain/utils/time_utils.hpp"
#include "shared/types/insights_errors.hpp"
#include "shared/utils/period_utils.hpp"

namespace tracer::core::application::use_cases::insights_query_support {

using tracer_core::core::dto::InsightsDisplayMode;
using tracer_core::core::dto::TemporalInsightsTargetsOutput;
using tracer_core::core::dto::TemporalSelectionKind;
using tracer_core::core::dto::TemporalSelectionPayload;
using tracer_core::core::dto::TemporalStructuredInsightsOutput;
using tracer_core::core::dto::TemporalStructuredInsightsQueryRequest;
using tracer_core::core::dto::TextOutput;
namespace core_api_failure = tracer::core::application::use_cases::failure;
namespace insights_api_support =
    tracer::core::application::use_cases::insights_support;

namespace {

auto BuildWindowMetadata(const PeriodInsightsData& insights)
    -> tracer_core::core::dto::InsightsWindowMetadata {
  return {.has_records = insights.has_records,
          .matched_day_count = insights.matched_day_count,
          .matched_record_count = insights.matched_record_count,
          .start_date = insights.start_date,
          .end_date = insights.end_date,
          .requested_days = insights.requested_days};
}

auto CopyRangeFields(const RangeInsightsData& source, RangeInsightsData& target)
    -> void {
  target.range_label = source.range_label;
  target.start_date = source.start_date;
  target.end_date = source.end_date;
  target.has_records = source.has_records;
  target.matched_day_count = source.matched_day_count;
  target.matched_record_count = source.matched_record_count;
  target.requested_days = source.requested_days;
  target.total_duration = source.total_duration;
  target.actual_days = source.actual_days;
  target.status_true_days = source.status_true_days;
  target.exercise_true_days = source.exercise_true_days;
  target.cardio_true_days = source.cardio_true_days;
  target.anaerobic_true_days = source.anaerobic_true_days;
  target.statuses = source.statuses;
  target.is_valid = source.is_valid;
  target.project_stats = source.project_stats;
  target.project_tree = source.project_tree;
}

auto WrapMonthlyInsights(const PeriodInsightsData& source) -> MonthlyInsightsData {
  MonthlyInsightsData out;
  CopyRangeFields(source, out);
  if (out.range_label.empty() && out.start_date.size() >= 7U) {
    out.range_label = out.start_date.substr(0, 7);
  }
  return out;
}

auto WrapWeeklyInsights(const PeriodInsightsData& source) -> WeeklyInsightsData {
  WeeklyInsightsData out;
  CopyRangeFields(source, out);
  if (out.range_label.empty() && !out.start_date.empty()) {
    out.range_label = FormatIsoWeek(IsoWeekFromDate(out.start_date));
  }
  return out;
}

auto WrapYearlyInsights(const PeriodInsightsData& source) -> YearlyInsightsData {
  YearlyInsightsData out;
  CopyRangeFields(source, out);
  if (out.range_label.empty() && out.start_date.size() >= 4U) {
    out.range_label = out.start_date.substr(0, 4);
  }
  return out;
}

auto ParseIsoDate(std::string_view value) -> std::chrono::sys_days {
  const std::string kNormalized = NormalizeDateArgument(value);
  const int kYear = std::stoi(kNormalized.substr(0, 4));
  const unsigned kMonth =
      static_cast<unsigned>(std::stoi(kNormalized.substr(5, 2)));
  const unsigned kDay =
      static_cast<unsigned>(std::stoi(kNormalized.substr(8, 2)));
  const std::chrono::year_month_day kYmd{std::chrono::year(kYear),
                                        std::chrono::month(kMonth),
                                        std::chrono::day(kDay)};
  if (!kYmd.ok()) {
    throw std::invalid_argument("Invalid ISO date: " + kNormalized);
  }
  return std::chrono::sys_days(kYmd);
}

auto FormatIsoDate(std::chrono::sys_days day) -> std::string {
  const std::chrono::year_month_day kYmd(day);
  std::ostringstream out;
  out << static_cast<int>(kYmd.year()) << '-';
  const unsigned kMonth = static_cast<unsigned>(kYmd.month());
  if (kMonth < 10U) {
    out << '0';
  }
  out << kMonth << '-';
  const unsigned kDayNumber = static_cast<unsigned>(kYmd.day());
  if (kDayNumber < 10U) {
    out << '0';
  }
  out << kDayNumber;
  return out.str();
}

}  // namespace

auto ToPeriodInsights(const RangeInsightsData& source) -> PeriodInsightsData {
  PeriodInsightsData out;
  CopyRangeFields(source, out);
  return out;
}

auto BuildTemporalStructuredInsightsFailure(std::string_view operation,
                                          InsightsDisplayMode display_mode,
                                          TemporalSelectionKind selection_kind,
                                          std::string_view details)
    -> TemporalStructuredInsightsOutput {
  return {
      .ok = false,
      .display_mode = display_mode,
      .selection_kind = selection_kind,
      .insights = DailyInsightsData{},
      .error_message = core_api_failure::BuildErrorMessage(operation, details)};
}

auto BuildTemporalStructuredInsightsFailure(
    std::string_view operation,
    const TemporalStructuredInsightsQueryRequest& request,
    std::string_view details) -> TemporalStructuredInsightsOutput {
  return BuildTemporalStructuredInsightsFailure(operation, request.display_mode,
                                              request.selection.kind, details);
}

auto BuildTemporalStructuredInsightsFailure(
    std::string_view operation,
    const TemporalStructuredInsightsQueryRequest& request,
    const std::exception& exception) -> TemporalStructuredInsightsOutput {
  return BuildTemporalStructuredInsightsFailure(operation, request,
                                              exception.what());
}

auto BuildTemporalTargetsFailure(std::string_view operation,
                                 InsightsDisplayMode display_mode,
                                 std::string_view details)
    -> TemporalInsightsTargetsOutput {
  return {
      .ok = false,
      .display_mode = display_mode,
      .items = {},
      .error_message = core_api_failure::BuildErrorMessage(operation, details)};
}

auto FormatTemporalStructuredInsights(
    const TemporalStructuredInsightsOutput& output, InsightsFormat format,
    std::string_view locale,
    tracer_core::application::ports::IInsightsDtoFormatter& formatter)
    -> TextOutput {
  switch (output.display_mode) {
    case InsightsDisplayMode::kDay: {
      const auto* insights = std::get_if<DailyInsightsData>(&output.insights);
      if (insights == nullptr) {
        return core_api_failure::BuildTextFailure(
            "RunTemporalInsightsQuery",
            "Temporal structured insights kind/data mismatch: day.");
      }
      return {
          .ok = true,
          .content = formatter.FormatDailyLocalized(*insights, format, locale),
          .error_message = ""};
    }
    case InsightsDisplayMode::kMonth: {
      const auto* insights = std::get_if<PeriodInsightsData>(&output.insights);
      if (insights == nullptr) {
        return core_api_failure::BuildTextFailure(
            "RunTemporalInsightsQuery",
            "Temporal structured insights kind/data mismatch: month.");
      }
      return {.ok = true,
              .content = formatter.FormatMonthlyLocalized(
                  WrapMonthlyInsights(*insights), format, locale),
              .error_message = ""};
    }
    case InsightsDisplayMode::kWeek: {
      const auto* insights = std::get_if<PeriodInsightsData>(&output.insights);
      if (insights == nullptr) {
        return core_api_failure::BuildTextFailure(
            "RunTemporalInsightsQuery",
            "Temporal structured insights kind/data mismatch: week.");
      }
      return {.ok = true,
              .content = formatter.FormatWeeklyLocalized(
                  WrapWeeklyInsights(*insights), format, locale),
              .error_message = ""};
    }
    case InsightsDisplayMode::kYear: {
      const auto* insights = std::get_if<PeriodInsightsData>(&output.insights);
      if (insights == nullptr) {
        return core_api_failure::BuildTextFailure(
            "RunTemporalInsightsQuery",
            "Temporal structured insights kind/data mismatch: year.");
      }
      return {.ok = true,
              .content = formatter.FormatYearlyLocalized(
                  WrapYearlyInsights(*insights), format, locale),
              .error_message = ""};
    }
    case InsightsDisplayMode::kRecent:
    case InsightsDisplayMode::kRange: {
      const auto* insights = std::get_if<PeriodInsightsData>(&output.insights);
      if (insights == nullptr) {
        return core_api_failure::BuildTextFailure(
            "RunTemporalInsightsQuery",
            "Temporal structured insights kind/data mismatch: period.");
      }
      return {
          .ok = true,
          .content = formatter.FormatPeriodLocalized(*insights, format, locale),
          .error_message = "",
          .insights_window_metadata = BuildWindowMetadata(*insights)};
    }
  }
  return core_api_failure::BuildTextFailure(
      "RunTemporalInsightsQuery",
      "Unhandled temporal structured insights display mode.");
}

auto NormalizeDateArgument(std::string_view argument) -> std::string {
  return NormalizeToDateFormat(std::string(argument));
}

auto NormalizeMonthArgument(std::string_view argument) -> std::string {
  return NormalizeToMonthFormat(std::string(argument));
}

auto ResolveMonthRange(std::string_view month_value)
    -> TemporalSelectionPayload {
  const std::string kNormalized = NormalizeMonthArgument(month_value);
  if (kNormalized.size() != 7U) {
    throw std::invalid_argument("Month argument must normalize to YYYY-MM.");
  }
  const int kYear = std::stoi(kNormalized.substr(0, 4));
  const unsigned kMonth =
      static_cast<unsigned>(std::stoi(kNormalized.substr(5, 2)));
  const auto kLastDay = std::chrono::year_month_day_last(
      std::chrono::year(kYear),
      std::chrono::month_day_last(std::chrono::month(kMonth)));
  std::ostringstream end_date;
  end_date << kYear << '-';
  if (kMonth < 10U) {
    end_date << '0';
  }
  end_date << kMonth << '-';
  const unsigned kLastDayNumber = static_cast<unsigned>(kLastDay.day());
  if (kLastDayNumber < 10U) {
    end_date << '0';
  }
  end_date << kLastDayNumber;
  return {.kind = TemporalSelectionKind::kDateRange,
          .start_date = kNormalized + "-01",
          .end_date = end_date.str()};
}

auto ResolveWeekRange(std::string_view week_value) -> TemporalSelectionPayload {
  IsoWeek week{};
  if (!ParseIsoWeek(week_value, week)) {
    throw std::invalid_argument(
        "Week argument must be in ISO week format (YYYY-Www or YYYYWww).");
  }
  return {.kind = TemporalSelectionKind::kDateRange,
          .start_date = IsoWeekStartDate(week),
          .end_date = IsoWeekEndDate(week)};
}

auto ResolveYearRange(std::string_view year_value) -> TemporalSelectionPayload {
  int year = 0;
  if (!ParseGregorianYear(year_value, year)) {
    throw std::invalid_argument("Year argument must be YYYY.");
  }
  return {.kind = TemporalSelectionKind::kDateRange,
          .start_date = std::to_string(year) + "-01-01",
          .end_date = std::to_string(year) + "-12-31"};
}

auto RequireSingleDaySelection(const TemporalSelectionPayload& selection)
    -> std::string {
  if (selection.kind != TemporalSelectionKind::kSingleDay) {
    throw tracer_core::common::InsightsContractError(
        "Temporal selection must be single_day for day display mode.",
        "insights.invalid_selection", "insights",
        {"Provide a single-day temporal selection for day insights."});
  }
  if (selection.anchor_date.has_value()) {
    throw tracer_core::common::InsightsContractError(
        "anchor_date is only supported for recent_days selection.",
        "insights.invalid_selection", "insights",
        {"Remove anchor_date or use recent display mode with recent_days."});
  }
  return NormalizeDateArgument(selection.date);
}

auto RequireDateRangeSelection(const TemporalSelectionPayload& selection)
    -> insights_api_support::DateRangeArgument {
  if (selection.kind != TemporalSelectionKind::kDateRange) {
    throw tracer_core::common::InsightsContractError(
        "Temporal selection must be date_range for this display mode.",
        "insights.invalid_selection", "insights",
        {"Provide a date-range temporal selection."});
  }
  if (selection.anchor_date.has_value()) {
    throw tracer_core::common::InsightsContractError(
        "anchor_date is only supported for recent_days selection.",
        "insights.invalid_selection", "insights",
        {"Remove anchor_date or use recent display mode with recent_days."});
  }
  return insights_api_support::ParseRangeArgument(selection.start_date + "|" +
                                                selection.end_date);
}

auto RequireRecentSelection(const TemporalSelectionPayload& selection)
    -> RecentSelection {
  if (selection.kind != TemporalSelectionKind::kRecentDays) {
    throw tracer_core::common::InsightsContractError(
        "Temporal selection must be recent_days for recent display mode.",
        "insights.invalid_selection", "insights",
        {"Provide a recent-days temporal selection for recent insights."});
  }
  RecentSelection out{.days = insights_api_support::ParseRecentDaysArgument(
                          std::to_string(selection.days)),
                      .anchor_date = std::nullopt};
  if (selection.anchor_date.has_value() && !selection.anchor_date->empty()) {
    out.anchor_date = NormalizeDateArgument(*selection.anchor_date);
  }
  return out;
}

auto ResolveAnchoredRecentInsights(
    tracer_core::application::ports::IInsightsDataQueryService& service,
    const RecentSelection& selection) -> PeriodInsightsData {
  const auto kAnchorDay = ParseIsoDate(*selection.anchor_date);
  const auto kStartDay = kAnchorDay - std::chrono::days(selection.days - 1);
  PeriodInsightsData insights =
      service.QueryRange(FormatIsoDate(kStartDay), FormatIsoDate(kAnchorDay));
  insights.range_label = std::to_string(selection.days) + " days";
  insights.start_date = FormatIsoDate(kStartDay);
  insights.end_date = FormatIsoDate(kAnchorDay);
  insights.requested_days = selection.days;
  return insights;
}

}  // namespace tracer::core::application::use_cases::insights_query_support
