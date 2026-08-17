#include "api/c_api/capabilities/insights/tracer_core_c_api_structured_insights_serializer.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include "application/ports/config/activity_hierarchy_toml_editor.hpp"

namespace tracer_core::core::c_api::insights {
namespace {

using ::ActivityRecordKind;
using ::DailyInsightsData;
using ::PeriodInsightsData;
using ::insights::ProjectNode;
using ::insights::ProjectTree;
using nlohmann::json;
using tracer_core::core::dto::InsightsDisplayMode;
using tracer_core::core::dto::TemporalSelectionKind;
using tracer_core::core::dto::TemporalStructuredInsightsOutput;
namespace fs = std::filesystem;
namespace config = tracer::core::application::config;

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

auto LoadParentColors(const fs::path& converter_config_toml_path)
    -> std::map<std::string, std::string> {
  const fs::path hierarchy_directory =
      converter_config_toml_path.parent_path() / "activity_hierarchy";
  std::map<std::string, std::string> colors;
  if (!fs::exists(hierarchy_directory) ||
      !fs::is_directory(hierarchy_directory)) {
    return colors;
  }

  for (const auto& entry :
       fs::recursive_directory_iterator(hierarchy_directory)) {
    if (!entry.is_regular_file() || entry.path().extension() != ".toml") {
      continue;
    }
    std::ifstream input(entry.path());
    if (!input) {
      throw std::runtime_error("Unable to read activity hierarchy TOML: " +
                               entry.path().string());
    }
    std::ostringstream toml_content;
    toml_content << input.rdbuf();
    const config::ActivityHierarchySnapshot hierarchy =
        config::DescribeActivityHierarchy(toml_content.str());
    if (hierarchy.color.has_value()) {
      colors.insert_or_assign(hierarchy.parent, *hierarchy.color);
    }
  }
  return colors;
}

auto FindParentColor(const std::string& project_path,
                     const std::map<std::string, std::string>& parent_colors)
    -> std::optional<std::string> {
  const std::string parent = project_path.substr(0, project_path.find('_'));
  const auto color = parent_colors.find(parent);
  return color == parent_colors.end()
             ? std::nullopt
             : std::optional<std::string>{color->second};
}

auto EncodeDetailedRecords(
    const DailyInsightsData& insights,
    const std::map<std::string, std::string>& parent_colors) -> json {
  json records = json::array();
  for (const auto& record : insights.detailed_records) {
    json output = {
        {"logical_id", record.logical_id},
        {"record_kind", EncodeRecordKind(record.kind)},
        {"start_time", record.start_time},
        {"end_time", record.end_time},
        {"project_path", record.project_path},
        {"duration_seconds", record.duration_seconds},
        {"activity_remark", record.activityRemark.value_or("")},
    };
    if (const auto parent_color =
            FindParentColor(record.project_path, parent_colors);
        parent_color.has_value()) {
      output["parent_color"] = *parent_color;
    }
    records.push_back(std::move(output));
  }
  return records;
}

auto EncodeDailyInsights(
    const DailyInsightsData& insights,
    const std::map<std::string, std::string>& parent_colors) -> json {
  json records = EncodeDetailedRecords(insights, parent_colors);

  json stats = json::object();
  for (const auto& [name, duration] : insights.stats) {
    stats[name] = duration;
  }

  json statuses = json::array();
  for (const auto& status : insights.metadata.statuses) {
    statuses.push_back(json{{"id", status.id},
                            {"label", status.label},
                            {"occurrence_count", status.occurrence_count},
                            {"total_duration", status.total_duration}});
  }

  return json{
      {"date", insights.date},
      {"metadata",
       {{"remark", insights.metadata.remark},
        {"getup_time", insights.metadata.getup_time},
        {"statuses", std::move(statuses)}}},
      {"total_duration", insights.activity.total_duration_seconds},
      {"project_stats", EncodeProjectStats(insights.project_stats)},
      {"detailed_records", std::move(records)},
      {"stats", std::move(stats)},
      {"project_tree", EncodeProjectTree(insights.project_tree)},
  };
}

auto EncodeActivityDays(const PeriodInsightsData& insights,
                        const std::map<std::string, std::string>& parent_colors)
    -> json {
  json days = json::array();
  for (const auto& day : insights.activity_days) {
    days.push_back(
        json{{"date", day.date},
             {"total_duration", day.activity.total_duration_seconds},
             {"detailed_records", EncodeDetailedRecords(day, parent_colors)}});
  }
  return days;
}

auto EncodePeriodInsights(
    const PeriodInsightsData& insights,
    const std::map<std::string, std::string>& parent_colors) -> json {
  json statuses = json::array();
  for (const auto& status : insights.statuses) {
    statuses.push_back(json{{"id", status.id},
                            {"label", status.label},
                            {"occurrence_count", status.occurrence_count},
                            {"total_duration", status.total_duration}});
  }
  return json{
      {"range_label", insights.range_label},
      {"start_date", insights.start_date},
      {"end_date", insights.end_date},
      {"requested_days", insights.requested_days},
      {"has_records", insights.has_records},
      {"matched_day_count", insights.matched_day_count},
      {"matched_record_count", insights.activity.occurrence_count},
      {"total_duration", insights.activity.total_duration_seconds},
      {"actual_days", insights.actual_days},
      {"statuses", std::move(statuses)},
      {"is_valid", insights.is_valid},
      {"project_stats", EncodeProjectStats(insights.project_stats)},
      {"project_tree", EncodeProjectTree(insights.project_tree)},
      {"activity_days", EncodeActivityDays(insights, parent_colors)},
  };
}

}  // namespace

auto SerializeTemporalStructuredInsights(
    const TemporalStructuredInsightsOutput& output,
    const fs::path& converter_config_toml_path) -> nlohmann::json {
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
    const auto parent_colors = LoadParentColors(converter_config_toml_path);
    if (const auto* daily = std::get_if<DailyInsightsData>(&output.insights);
        daily != nullptr) {
      payload["insights_kind"] = "day";
      payload["insights"] = EncodeDailyInsights(*daily, parent_colors);
    } else if (const auto* period =
                   std::get_if<PeriodInsightsData>(&output.insights);
               period != nullptr) {
      payload["insights_kind"] = "period";
      payload["insights"] = EncodePeriodInsights(*period, parent_colors);
    }
  }

  return payload;
}

}  // namespace tracer_core::core::c_api::insights
