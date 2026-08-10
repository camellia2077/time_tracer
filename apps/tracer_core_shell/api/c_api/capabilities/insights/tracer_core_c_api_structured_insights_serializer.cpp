#include "api/c_api/capabilities/insights/tracer_core_c_api_structured_insights_serializer.hpp"

#include <cstdint>
#include <string>
#include <utility>

namespace tracer_core::core::c_api::insights {
namespace {

using nlohmann::json;
using tracer_core::core::dto::InsightsDisplayMode;
using tracer_core::core::dto::TemporalSelectionKind;
using tracer_core::core::dto::TemporalStructuredInsightsOutput;
using ::ActivityRecordKind;
using ::DailyInsightsData;
using ::PeriodInsightsData;
using ::insights::ProjectNode;
using ::insights::ProjectTree;

auto ToWireValue(InsightsDisplayMode display_mode) -> std::string {
  switch (display_mode) {
    case InsightsDisplayMode::kDay:
      return "day";
    case InsightsDisplayMode::kWeek:
      return "week";
    case InsightsDisplayMode::kMonth:
      return "month";
    case InsightsDisplayMode::kYear:
      return "year";
    case InsightsDisplayMode::kRange:
      return "range";
    case InsightsDisplayMode::kRecent:
      return "recent";
  }
  return "day";
}

auto ToWireValue(TemporalSelectionKind selection_kind) -> std::string {
  switch (selection_kind) {
    case TemporalSelectionKind::kSingleDay:
      return "single_day";
    case TemporalSelectionKind::kDateRange:
      return "date_range";
    case TemporalSelectionKind::kRecentDays:
      return "recent_days";
  }
  return "single_day";
}

auto EncodeProjectNode(const ProjectNode& node) -> json {
  json children = json::object();
  for (const auto& [name, child] : node.children) {
    children[name] = EncodeProjectNode(child);
  }
  return json{{"duration", node.duration}, {"children", std::move(children)}};
}

auto EncodeProjectTree(const ProjectTree& tree) -> json {
  json output = json::object();
  for (const auto& [name, node] : tree) {
    output[name] = EncodeProjectNode(node);
  }
  return output;
}

auto EncodeProjectStats(
    const std::vector<std::pair<std::int64_t, std::int64_t>>& stats) -> json {
  json output = json::array();
  for (const auto& [start, duration] : stats) {
    output.push_back(json{{"start", start}, {"duration", duration}});
  }
  return output;
}

auto EncodeRecordKind(ActivityRecordKind kind) -> std::string {
  switch (kind) {
    case ActivityRecordKind::kInterval:
      return "interval";
    case ActivityRecordKind::kEndOnly:
      return "end_only";
  }
  return "interval";
}

auto EncodeDailyInsights(const DailyInsightsData& insights) -> json {
  json records = json::array();
  for (const auto& record : insights.detailed_records) {
    records.push_back(json{
        {"logical_id", record.logical_id},
        {"record_kind", EncodeRecordKind(record.kind)},
        {"start_time", record.start_time},
        {"end_time", record.end_time},
        {"project_path", record.project_path},
        {"duration_seconds", record.duration_seconds},
        {"activity_remark", record.activityRemark.value_or("")},
    });
  }

  json stats = json::object();
  for (const auto& [name, duration] : insights.stats) {
    stats[name] = duration;
  }

  json statuses = json::array();
  for (const auto& status : insights.metadata.statuses) {
    statuses.push_back(json{{"id", status.id},
                            {"label", status.label},
                            {"value", status.value}});
  }

  return json{
      {"date", insights.date},
      {"metadata",
       {{"remark", insights.metadata.remark},
        {"getup_time", insights.metadata.getup_time},
        {"statuses", std::move(statuses)}}},
      {"total_duration", insights.total_duration},
      {"project_stats", EncodeProjectStats(insights.project_stats)},
      {"detailed_records", std::move(records)},
      {"stats", std::move(stats)},
      {"project_tree", EncodeProjectTree(insights.project_tree)},
  };
}

auto EncodePeriodInsights(const PeriodInsightsData& insights) -> json {
  return json{
      {"range_label", insights.range_label},
      {"start_date", insights.start_date},
      {"end_date", insights.end_date},
      {"requested_days", insights.requested_days},
      {"has_records", insights.has_records},
      {"matched_day_count", insights.matched_day_count},
      {"matched_record_count", insights.matched_record_count},
      {"total_duration", insights.total_duration},
      {"actual_days", insights.actual_days},
      {"status_true_days", insights.status_true_days},
      {"exercise_true_days", insights.exercise_true_days},
      {"cardio_true_days", insights.cardio_true_days},
      {"anaerobic_true_days", insights.anaerobic_true_days},
      {"is_valid", insights.is_valid},
      {"project_stats", EncodeProjectStats(insights.project_stats)},
      {"project_tree", EncodeProjectTree(insights.project_tree)},
  };
}

}  // namespace

auto SerializeTemporalStructuredInsights(
    const TemporalStructuredInsightsOutput& output) -> nlohmann::json {
  json payload = {
      {"ok", output.ok},
      {"display_mode", ToWireValue(output.display_mode)},
      {"selection_kind", ToWireValue(output.selection_kind)},
      {"error_message", output.error_message},
      {"error_code", output.error_contract.error_code},
      {"error_category", output.error_contract.error_category},
      {"hints", output.error_contract.hints},
  };

  if (output.ok) {
    if (const auto* daily = std::get_if<DailyInsightsData>(&output.insights);
        daily != nullptr) {
      payload["insights_kind"] = "day";
      payload["insights"] = EncodeDailyInsights(*daily);
    } else if (const auto* period =
                   std::get_if<PeriodInsightsData>(&output.insights);
               period != nullptr) {
      payload["insights_kind"] = "period";
      payload["insights"] = EncodePeriodInsights(*period);
    }
  }

  return payload;
}

}  // namespace tracer_core::core::c_api::insights
