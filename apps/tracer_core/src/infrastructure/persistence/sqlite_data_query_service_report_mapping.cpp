// infrastructure/persistence/sqlite_data_query_service_report_mapping.cpp
#include <nlohmann/json.hpp>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "infrastructure/config/file_converter_config_provider.hpp"
#include "infrastructure/persistence/sqlite_data_query_service_internal.hpp"
#include "infrastructure/query/data/data_query_models.hpp"
#include "infrastructure/query/data/data_query_repository.hpp"
#include "infrastructure/query/data/orchestrators/date_range_resolver.hpp"
#include "infrastructure/query/data/stats/report_chart_stats_calculator.hpp"

namespace infra_data_query = tracer_core::infrastructure::query::data;
namespace infra_data_query_orchestrators =
    tracer_core::infrastructure::query::data::orchestrators;
namespace infra_data_query_stats =
    tracer_core::infrastructure::query::data::stats;

namespace infrastructure::persistence::data_query_service_internal {
namespace {

constexpr int kDefaultReportChartLookbackDays = 7;

using nlohmann::json;

auto ResolveRequestedRootFilter(
    const tracer_core::core::dto::DataQueryRequest& request)
    -> std::optional<std::string> {
  const auto kNormalizedRoot = NormalizeProjectRootFilter(request.root);
  if (kNormalizedRoot.has_value()) {
    return kNormalizedRoot;
  }
  return NormalizeProjectRootFilter(request.project);
}

}  // namespace

auto BuildMappingNamesContent(
    const std::optional<std::filesystem::path>& converter_config_toml_path)
    -> std::string {
  if (!converter_config_toml_path.has_value() ||
      converter_config_toml_path->empty()) {
    throw std::runtime_error(
        "mapping_names query requires converter config path.");
  }

  infrastructure::config::FileConverterConfigProvider config_provider(
      *converter_config_toml_path,
      std::unordered_map<std::filesystem::path, std::filesystem::path>{});
  const ConverterConfig kConfig = config_provider.LoadConverterConfig();

  std::set<std::string> names;
  for (const auto& [alias, full_name] : kConfig.text_mapping) {
    const std::string kTrimmedAlias = TrimCopy(alias);
    const std::string kTrimmedFullName = TrimCopy(full_name);
    if (!kTrimmedAlias.empty()) {
      names.insert(kTrimmedAlias);
    }
    if (!kTrimmedFullName.empty()) {
      names.insert(kTrimmedFullName);
    }
  }

  json payload = json::object();
  payload["names"] = json::array();
  for (const auto& name : names) {
    payload["names"].push_back(name);
  }
  return payload.dump();
}

auto BuildReportChartContent(
    sqlite3* db_conn, const tracer_core::core::dto::DataQueryRequest& request)
    -> std::string {
  const auto kSelectedRoot = ResolveRequestedRootFilter(request);
  const std::vector<std::string> kRoots =
      infra_data_query::QueryProjectRootNames(db_conn);

  const int kPayloadLookbackDays = ResolvePositiveLookbackDays(
      request.lookback_days, kDefaultReportChartLookbackDays,
      "--lookback-days");
  const auto kExplicitRange =
      infra_data_query_orchestrators::ResolveExplicitDateRange(
          request.from_date, request.to_date,
          "report-chart requires both --from-date and --to-date.",
          "report-chart invalid range: from_date must be <= to_date.",
          "report-chart resolved invalid date range.");

  json payload = json::object();
  payload["roots"] = kRoots;
  payload["selected_root"] = kSelectedRoot.value_or("");
  payload["lookback_days"] = kPayloadLookbackDays;
  payload["average_duration_seconds"] = 0;
  payload["total_duration_seconds"] = 0;
  payload["active_days"] = 0;
  payload["range_days"] = 0;
  if (kExplicitRange.has_value()) {
    payload["from_date"] = kExplicitRange->start_date;
    payload["to_date"] = kExplicitRange->end_date;
  }
  payload["series"] = json::array();

  const auto kAnyTrackedDate =
      infra_data_query::QueryLatestTrackedDate(db_conn);
  if (!kAnyTrackedDate.has_value()) {
    return payload.dump();
  }

  infra_data_query_orchestrators::ResolvedDateRange range;
  if (kExplicitRange.has_value()) {
    range = *kExplicitRange;
  } else {
    range = infra_data_query_orchestrators::ResolveRollingDateRange(
        kPayloadLookbackDays);
    payload["from_date"] = range.start_date;
    payload["to_date"] = range.end_date;
  }
  infra_data_query_orchestrators::ValidateDateRange(
      range.start_date, range.end_date,
      "report-chart invalid range: from_date must be <= to_date.",
      "report-chart resolved invalid date range.");

  const std::vector<infra_data_query::DayDurationRow> kSparseRows =
      infra_data_query::QueryDayDurationsByRootInDateRange(
          db_conn, kSelectedRoot, range.start_date, range.end_date);
  const auto kSeriesResult = infra_data_query_stats::BuildReportChartSeries(
      range.start_date, range.end_date, kSparseRows);
  for (const auto& point : kSeriesResult.series) {
    payload["series"].push_back(json{
        {"date", point.date},
        {"duration_seconds", point.duration_seconds},
        {"epoch_day", point.epoch_day},
    });
  }
  payload["average_duration_seconds"] =
      kSeriesResult.stats.average_duration_seconds;
  payload["total_duration_seconds"] =
      kSeriesResult.stats.total_duration_seconds;
  payload["active_days"] = kSeriesResult.stats.active_days;
  payload["range_days"] = kSeriesResult.stats.range_days;

  return payload.dump();
}

}  // namespace infrastructure::persistence::data_query_service_internal
