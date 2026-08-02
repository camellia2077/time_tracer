#include "api/c_api/capabilities/reporting/tracer_core_c_api_structured_report_serializer.hpp"

#include <cstdint>
#include <string>
#include <utility>

namespace tracer_core::core::c_api::reporting {
namespace {

using nlohmann::json;
using tracer_core::core::dto::ReportDisplayMode;
using tracer_core::core::dto::TemporalSelectionKind;
using tracer_core::core::dto::TemporalStructuredReportOutput;
using ::ActivityRecordKind;
using ::DailyReportData;
using ::PeriodReportData;
using ::reporting::ProjectNode;
using ::reporting::ProjectTree;

auto ToWireValue(ReportDisplayMode display_mode) -> std::string {
  switch (display_mode) {
    case ReportDisplayMode::kDay:
      return "day";
    case ReportDisplayMode::kWeek:
      return "week";
    case ReportDisplayMode::kMonth:
      return "month";
    case ReportDisplayMode::kYear:
      return "year";
    case ReportDisplayMode::kRange:
      return "range";
    case ReportDisplayMode::kRecent:
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

auto EncodeDailyReport(const DailyReportData& report) -> json {
  json records = json::array();
  for (const auto& record : report.detailed_records) {
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
  for (const auto& [name, duration] : report.stats) {
    stats[name] = duration;
  }

  json statuses = json::array();
  for (const auto& status : report.metadata.statuses) {
    statuses.push_back(json{{"id", status.id},
                            {"label", status.label},
                            {"value", status.value}});
  }

  return json{
      {"date", report.date},
      {"metadata",
       {{"remark", report.metadata.remark},
        {"getup_time", report.metadata.getup_time},
        {"statuses", std::move(statuses)}}},
      {"total_duration", report.total_duration},
      {"project_stats", EncodeProjectStats(report.project_stats)},
      {"detailed_records", std::move(records)},
      {"stats", std::move(stats)},
      {"project_tree", EncodeProjectTree(report.project_tree)},
  };
}

auto EncodePeriodReport(const PeriodReportData& report) -> json {
  return json{
      {"range_label", report.range_label},
      {"start_date", report.start_date},
      {"end_date", report.end_date},
      {"requested_days", report.requested_days},
      {"has_records", report.has_records},
      {"matched_day_count", report.matched_day_count},
      {"matched_record_count", report.matched_record_count},
      {"total_duration", report.total_duration},
      {"actual_days", report.actual_days},
      {"status_true_days", report.status_true_days},
      {"exercise_true_days", report.exercise_true_days},
      {"cardio_true_days", report.cardio_true_days},
      {"anaerobic_true_days", report.anaerobic_true_days},
      {"is_valid", report.is_valid},
      {"project_stats", EncodeProjectStats(report.project_stats)},
      {"project_tree", EncodeProjectTree(report.project_tree)},
  };
}

}  // namespace

auto SerializeTemporalStructuredReport(
    const TemporalStructuredReportOutput& output) -> nlohmann::json {
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
    if (const auto* daily = std::get_if<DailyReportData>(&output.report);
        daily != nullptr) {
      payload["report_kind"] = "day";
      payload["report"] = EncodeDailyReport(*daily);
    } else if (const auto* period =
                   std::get_if<PeriodReportData>(&output.report);
               period != nullptr) {
      payload["report_kind"] = "period";
      payload["report"] = EncodePeriodReport(*period);
    }
  }

  return payload;
}

}  // namespace tracer_core::core::c_api::reporting
