// infra/query/data/repository/query_runtime_service_report_mapping.cpp
#include <nlohmann/json.hpp>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <optional>
#include <numeric>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "domain/reports/models/project_tree.hpp"
#include "infra/config/loader/alias_mapping_index_utils.hpp"
#include "infra/config/loader/toml_loader_utils.hpp"
#include "infra/query/data/internal/report_mapping.hpp"

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
namespace modloader = tracer::core::infrastructure::config::loader;

namespace tracer::core::infrastructure::query::data::internal {
namespace {

constexpr int kDefaultReportChartLookbackDays = 7;
constexpr int kDefaultReportCompositionLookbackDays = 7;

using nlohmann::json;

struct ResolvedReportQueryWindow {
  int payload_lookback_days = 0;
  std::optional<infra_data_query_orchestrators::ResolvedDateRange> explicit_range;
  infra_data_query_orchestrators::ResolvedDateRange range;
};


[[nodiscard]] auto BuildCompositionTreeNodePayload(
    std::string_view name, const reporting::ProjectNode& node) -> json {
  json payload = {
      {"name", name},
      {"duration_seconds", node.duration},
      {"occurrence_count", node.occurrence_count},
      {"children", json::array()},
  };

  std::vector<std::pair<std::string_view, const reporting::ProjectNode*>>
      children;
  children.reserve(node.children.size());
  for (const auto& [child_name, child] : node.children) {
    if (!child_name.empty() && child.occurrence_count > 0) {
      children.push_back({child_name, &child});
    }
  }
  std::ranges::sort(
      children, [](const auto& left, const auto& right) {
        return left.first < right.first;
      });
  for (const auto& [child_name, child] : children) {
    payload["children"].push_back(
        BuildCompositionTreeNodePayload(child_name, *child));
  }
  return payload;
}

[[nodiscard]] auto BuildCompositionTreePayload(
    const reporting::ProjectTree& tree) -> json {
  json payload = json::array();
  std::vector<std::pair<std::string_view, const reporting::ProjectNode*>>
      roots;
  roots.reserve(tree.size());
  for (const auto& [root_name, root] : tree) {
    if (!root_name.empty() && root.occurrence_count > 0) {
      roots.push_back({root_name, &root});
    }
  }
  std::ranges::sort(roots, [](const auto& left, const auto& right) {
    return left.first < right.first;
  });
  for (const auto& [root_name, root] : roots) {
    payload.push_back(BuildCompositionTreeNodePayload(root_name, *root));
  }
  return payload;
}

auto ParseIsoDateOrThrow(std::string_view value) -> std::chrono::sys_days {
  if (value.size() != 10 || value[4] != '-' || value[7] != '-') {
    throw std::runtime_error("Invalid ISO date boundary: " + std::string(value));
  }

  const int year = std::stoi(std::string(value.substr(0, 4)));
  const unsigned month =
      static_cast<unsigned>(std::stoi(std::string(value.substr(5, 2))));
  const unsigned day =
      static_cast<unsigned>(std::stoi(std::string(value.substr(8, 2))));
  const std::chrono::year_month_day ymd{
      std::chrono::year{year}, std::chrono::month{month},
      std::chrono::day{day}};
  if (!ymd.ok()) {
    throw std::runtime_error("Invalid ISO date boundary: " + std::string(value));
  }
  return std::chrono::sys_days{ymd};
}

auto InclusiveRangeDays(std::string_view start_date, std::string_view end_date)
    -> int {
  const auto start = ParseIsoDateOrThrow(start_date);
  const auto end = ParseIsoDateOrThrow(end_date);
  return static_cast<int>((end - start).count()) + 1;
}

auto LoadConverterConfigOrThrow(
    const std::optional<std::filesystem::path>& converter_config_toml_path,
    std::string_view query_name) -> modtypes::ConverterConfig {
  if (!converter_config_toml_path.has_value() ||
      converter_config_toml_path->empty()) {
    throw std::runtime_error(std::string(query_name) +
                             " query requires converter config path.");
  }

  FileConverterConfigProvider config_provider(
      *converter_config_toml_path,
      std::unordered_map<std::filesystem::path, std::filesystem::path>{});
  return config_provider.LoadConverterConfig();
}

auto BuildNamesPayload(const std::set<std::string>& names) -> std::string {
  json payload = json::object();
  payload["names"] = json::array();
  for (const auto& name : names) {
    payload["names"].push_back(name);
  }
  return payload.dump();
}

auto BuildAliasEntriesPayload(
    const std::vector<modloader::detail::ExpandedAliasMappingEntry>& entries)
    -> std::string {
  json payload = json::object();
  payload["entries"] = json::array();
  for (const auto& entry : entries) {
    payload["entries"].push_back(json{
        {"alias", entry.alias_key},
        {"canonical", entry.canonical_value},
    });
  }
  return payload.dump();
}

auto ResolveRequestedRootFilter(
    const tracer_core::core::dto::DataQueryRequest& request)
    -> std::optional<std::string> {
  const auto kNormalizedRoot = NormalizeProjectRootFilter(request.root);
  if (kNormalizedRoot.has_value()) {
    return kNormalizedRoot;
  }
  return NormalizeProjectRootFilter(request.project);
}

auto ResolveReportQueryWindow(
    const tracer_core::core::dto::DataQueryRequest& request,
    int default_lookback_days,
    const infra_data_query_orchestrators::ExplicitDateRangeErrors&
        explicit_range_errors)
    -> ResolvedReportQueryWindow {
  const int payload_lookback_days =
      ResolvePositiveLookbackDays(request.lookback_days, default_lookback_days,
                                  "--lookback-days");
  const auto explicit_range =
      infra_data_query_orchestrators::ResolveExplicitDateRange(
          request.from_date, request.to_date, explicit_range_errors);

  ResolvedReportQueryWindow window{
      .payload_lookback_days = payload_lookback_days,
      .explicit_range = explicit_range,
      .range = explicit_range.value_or(
          infra_data_query_orchestrators::ResolveRollingDateRange(
              payload_lookback_days)),
  };
  infra_data_query_orchestrators::ValidateDateRange(
      infra_data_query_orchestrators::DateRangeBoundaries{
          .start_date = window.range.start_date,
          .end_date = window.range.end_date,
      },
      explicit_range_errors.validation);
  return window;
}

}  // namespace

auto BuildMappingNamesContent(
    const std::optional<std::filesystem::path>& converter_config_toml_path)
    -> std::string {
  const modtypes::ConverterConfig kConfig = LoadConverterConfigOrThrow(
      converter_config_toml_path, "mapping_names");

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
  return BuildNamesPayload(names);
}

auto BuildActivityAliasMappingsContent(
    const std::optional<std::filesystem::path>& converter_config_toml_path)
    -> std::string {
  if (!converter_config_toml_path.has_value() ||
      converter_config_toml_path->empty()) {
    throw std::runtime_error(
        "activity_alias_mappings query requires converter config path.");
  }

  const std::filesystem::path config_path = *converter_config_toml_path;
  const std::filesystem::path config_dir = config_path.parent_path();
  const auto definition = modloader::detail::LoadAliasMappingDefinition(
      config_dir / "activity_hierarchy", [](const std::filesystem::path& path) {
        return modloader::ReadToml(path);
      });
  return BuildAliasEntriesPayload(definition.expanded_entries);
}

auto BuildMappingAliasKeysContent(
    const std::optional<std::filesystem::path>& converter_config_toml_path)
    -> std::string {
  const modtypes::ConverterConfig kConfig = LoadConverterConfigOrThrow(
      converter_config_toml_path, "mapping_alias_keys");

  std::set<std::string> alias_keys;
  for (const auto& [alias, full_name] : kConfig.text_mapping) {
    static_cast<void>(full_name);
    const std::string kTrimmedAlias = TrimCopy(alias);
    if (!kTrimmedAlias.empty()) {
      alias_keys.insert(kTrimmedAlias);
    }
  }
  return BuildNamesPayload(alias_keys);
}

auto BuildWakeKeywordsContent(
    const std::optional<std::filesystem::path>& converter_config_toml_path)
    -> std::string {
  const modtypes::ConverterConfig kConfig = LoadConverterConfigOrThrow(
      converter_config_toml_path, "wake_keywords");

  std::set<std::string> wake_keywords;
  for (const auto& wake_keyword : kConfig.sleep_inference.wake_keywords) {
    const std::string kTrimmedKeyword = TrimCopy(wake_keyword);
    if (!kTrimmedKeyword.empty()) {
      wake_keywords.insert(kTrimmedKeyword);
    }
  }
  return BuildNamesPayload(wake_keywords);
}

auto BuildAuthorableEventTokensContent(
    const std::optional<std::filesystem::path>& converter_config_toml_path)
    -> std::string {
  const modtypes::ConverterConfig kConfig = LoadConverterConfigOrThrow(
      converter_config_toml_path, "authorable_event_tokens");

  std::set<std::string> authorable_tokens;
  for (const auto& [alias, full_name] : kConfig.text_mapping) {
    const std::string kTrimmedAlias = TrimCopy(alias);
    const std::string kTrimmedCanonical = TrimCopy(full_name);
    if (!kTrimmedAlias.empty()) {
      authorable_tokens.insert(kTrimmedAlias);
    }
    if (!kTrimmedCanonical.empty()) {
      authorable_tokens.insert(kTrimmedCanonical);
    }
  }
  for (const auto& wake_keyword : kConfig.sleep_inference.wake_keywords) {
    const std::string kTrimmedKeyword = TrimCopy(wake_keyword);
    if (!kTrimmedKeyword.empty()) {
      authorable_tokens.insert(kTrimmedKeyword);
    }
  }
  return BuildNamesPayload(authorable_tokens);
}

auto ValidateReportChartRequest(
    const tracer_core::core::dto::DataQueryRequest& request) -> void {
  static_cast<void>(ResolvePositiveLookbackDays(request.lookback_days,
                                                kDefaultReportChartLookbackDays,
                                                "--lookback-days"));

  const infra_data_query_orchestrators::ExplicitDateRangeErrors kRangeErrors{
      .missing_boundary_error =
          "report-chart requires both --from-date and --to-date.",
      .validation =
          {
              .invalid_range_error =
                  "report-chart invalid range: from_date must be <= to_date.",
              .invalid_date_error = "report-chart resolved invalid date range.",
          },
  };
  static_cast<void>(infra_data_query_orchestrators::ResolveExplicitDateRange(
      request.from_date, request.to_date, kRangeErrors));
}

auto ValidateReportCompositionRequest(
    const tracer_core::core::dto::DataQueryRequest& request) -> void {
  static_cast<void>(ResolvePositiveLookbackDays(
      request.lookback_days, kDefaultReportCompositionLookbackDays,
      "--lookback-days"));

  const infra_data_query_orchestrators::ExplicitDateRangeErrors kRangeErrors{
      .missing_boundary_error =
          "report-composition requires both --from-date and --to-date.",
      .validation =
          {
              .invalid_range_error =
                  "report-composition invalid range: from_date must be <= to_date.",
              .invalid_date_error =
                  "report-composition resolved invalid date range.",
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
  const infra_data_query_orchestrators::ExplicitDateRangeErrors kRangeErrors{
      .missing_boundary_error =
          "report-chart requires both --from-date and --to-date.",
      .validation =
          {
              .invalid_range_error =
                  "report-chart invalid range: from_date must be <= to_date.",
              .invalid_date_error = "report-chart resolved invalid date range.",
          },
  };
  const auto kWindow = ResolveReportQueryWindow(
      request, kDefaultReportChartLookbackDays, kRangeErrors);

  json payload = json::object();
  payload["roots"] = kRoots;
  payload["selected_root"] = kSelectedRoot.value_or("");
  payload["lookback_days"] = kWindow.payload_lookback_days;
  payload["average_duration_seconds"] = 0;
  payload["total_duration_seconds"] = 0;
  payload["active_days"] = 0;
  payload["range_days"] = 0;
  if (kWindow.explicit_range.has_value()) {
    payload["from_date"] = kWindow.explicit_range->start_date;
    payload["to_date"] = kWindow.explicit_range->end_date;
  }
  payload["series"] = json::array();

  const auto kAnyTrackedDate =
      infra_data_query::QueryLatestTrackedDate(db_conn);
  if (!kAnyTrackedDate.has_value()) {
    return payload.dump();
  }

  const auto& range = kWindow.range;
  if (!kWindow.explicit_range.has_value()) {
    payload["from_date"] = range.start_date;
    payload["to_date"] = range.end_date;
  }

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

auto BuildReportCompositionContent(
    sqlite3* db_conn, const tracer_core::core::dto::DataQueryRequest& request)
    -> std::string {
  ValidateReportCompositionRequest(request);
  const infra_data_query_orchestrators::ExplicitDateRangeErrors kRangeErrors{
      .missing_boundary_error =
          "report-composition requires both --from-date and --to-date.",
      .validation =
          {
              .invalid_range_error =
                  "report-composition invalid range: from_date must be <= to_date.",
              .invalid_date_error =
                  "report-composition resolved invalid date range.",
          },
  };
  const auto kWindow = ResolveReportQueryWindow(
      request, kDefaultReportCompositionLookbackDays, kRangeErrors);

  json payload = json::object();
  payload["lookback_days"] = kWindow.payload_lookback_days;
  payload["total_duration_seconds"] = 0;
  payload["active_root_count"] = 0;
  payload["range_days"] = 0;
  payload["tree"] = json::array();
  if (kWindow.explicit_range.has_value()) {
    payload["from_date"] = kWindow.explicit_range->start_date;
    payload["to_date"] = kWindow.explicit_range->end_date;
  }

  const auto kAnyTrackedDate =
      infra_data_query::QueryLatestTrackedDate(db_conn);
  if (!kAnyTrackedDate.has_value()) {
    return payload.dump();
  }

  const auto& range = kWindow.range;
  if (!kWindow.explicit_range.has_value()) {
    payload["from_date"] = range.start_date;
    payload["to_date"] = range.end_date;
  }

  infra_data_query::QueryFilters filters;
  filters.from_date = range.start_date;
  filters.to_date = range.end_date;

  // Composition reuses the same temporal window resolution as trend charts,
  // but aggregates root totals across the full period instead of per-day series.
  const reporting::ProjectTree tree =
      infra_data_query::QueryProjectTree(db_conn, filters);
  const int kRangeDays =
      InclusiveRangeDays(range.start_date, range.end_date);
  const std::int64_t total_duration_seconds = std::accumulate(
      tree.begin(), tree.end(), static_cast<std::int64_t>(0),
      [](std::int64_t total, const auto& entry) {
        const auto& [root_name, node] = entry;
        return total + (!root_name.empty() && node.duration > 0 ? node.duration : 0);
      });
  const int active_root_count = static_cast<int>(std::count_if(
      tree.begin(), tree.end(), [](const auto& entry) {
        const auto& [root_name, node] = entry;
        return !root_name.empty() && node.duration > 0;
      }));
  payload["total_duration_seconds"] = total_duration_seconds;
  payload["active_root_count"] = active_root_count;
  payload["range_days"] = kRangeDays;
  // The complete weighted activity tree is the sole composition payload.
  // Every client derives its current visual layer from a tree node's children.
  payload["tree"] = BuildCompositionTreePayload(tree);

  return payload.dump();
}

}  // namespace tracer::core::infrastructure::query::data::internal
