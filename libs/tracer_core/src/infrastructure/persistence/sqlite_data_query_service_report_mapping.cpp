// infrastructure/persistence/sqlite_data_query_service_report_mapping.cpp
#include <nlohmann/json.hpp>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "infrastructure/query/data/internal/report_mapping.hpp"

import tracer.core.infrastructure.config.file_converter_config_provider;
import tracer.core.domain.types.converter_config;
import tracer.core.infrastructure.query.data.orchestrators.date_range_resolver;
import tracer.core.infrastructure.query.data.repository;
import tracer.core.infrastructure.query.data.stats;

namespace infra_data_query = tracer::core::infrastructure::query::data;
namespace infra_data_query_orchestrators =
    tracer::core::infrastructure::query::data::orchestrators;
namespace infra_data_query_stats =
    tracer::core::infrastructure::query::data::stats;
namespace modtypes = tracer::core::domain::modtypes;
using FileConverterConfigProvider =
    tracer::core::infrastructure::config::FileConverterConfigProvider;

namespace tracer::core::infrastructure::query::data::internal {
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

  FileConverterConfigProvider config_provider(
      *converter_config_toml_path,
      std::unordered_map<std::filesystem::path, std::filesystem::path>{});
  const modtypes::ConverterConfig kConfig =
      config_provider.LoadConverterConfig();

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

auto ValidateReportChartRequest(
    const tracer_core::core::dto::DataQueryRequest& request) -> void {
  static_cast<void>(ResolvePositiveLookbackDays(
      request.lookback_days, kDefaultReportChartLookbackDays,
      "--lookback-days"));

  const infra_data_query_orchestrators::ExplicitDateRangeErrors kRangeErrors{
      .missing_boundary_error =
          "report-chart requires both --from-date and --to-date.",
      .validation =
          {
              .invalid_range_error =
                  "report-chart invalid range: from_date must be <= to_date.",
              .invalid_date_error =
                  "report-chart resolved invalid date range.",
          },
  };
  static_cast<void>(infra_data_query_orchestrators::ResolveExplicitDateRange(
      request.from_date, request.to_date, kRangeErrors));
}

auto BuildReportChartContent(
    sqlite3* db_conn, const tracer_core::core::dto::DataQueryRequest& request)
    -> std::string {
  const auto kSelectedRoot = ResolveRequestedRootFilter(request);
  const std::vector<std::string> kRoots =
      infra_data_query::QueryProjectRootNames(db_conn);

  ValidateReportChartRequest(request);
  const int kPayloadLookbackDays = ResolvePositiveLookbackDays(
      request.lookback_days, kDefaultReportChartLookbackDays,
      "--lookback-days");
  const infra_data_query_orchestrators::ExplicitDateRangeErrors kRangeErrors{
      .missing_boundary_error =
          "report-chart requires both --from-date and --to-date.",
      .validation =
          {
              .invalid_range_error =
                  "report-chart invalid range: from_date must be <= to_date.",
              .invalid_date_error =
                  "report-chart resolved invalid date range.",
          },
  };
  const auto kExplicitRange =
      infra_data_query_orchestrators::ResolveExplicitDateRange(
          request.from_date, request.to_date, kRangeErrors);

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
      infra_data_query_orchestrators::DateRangeBoundaries{
          .start_date = range.start_date,
          .end_date = range.end_date,
      },
      kRangeErrors.validation);

  const std::vector<infra_data_query::DayDurationRow> kSparseRows =
      infra_data_query::QueryDayDurationsByRootInDateRange(
          db_conn, kSelectedRoot, range.start_date, range.end_date);
  const auto kSeriesResult = infra_data_query_stats::BuildReportChartSeries(
      {.start_date = range.start_date, .end_date = range.end_date},
      kSparseRows);
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

}  // namespace tracer::core::infrastructure::query::data::internal
