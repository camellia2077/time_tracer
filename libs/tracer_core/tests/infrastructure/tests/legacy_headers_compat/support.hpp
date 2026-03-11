#ifndef TRACER_CORE_TESTS_INFRASTRUCTURE_LEGACY_HEADERS_COMPAT_SUPPORT_HPP_
#define TRACER_CORE_TESTS_INFRASTRUCTURE_LEGACY_HEADERS_COMPAT_SUPPORT_HPP_

#include "application/ports/i_platform_clock.hpp"
#include "infrastructure/config/config_loader.hpp"
#include "infrastructure/config/file_converter_config_provider.hpp"
#include "infrastructure/config/internal/config_detail_loader.hpp"
#include "infrastructure/config/internal/config_parser_utils.hpp"
#include "infrastructure/config/loader/converter_config_loader.hpp"
#include "infrastructure/config/loader/report_config_loader.hpp"
#include "infrastructure/config/loader/toml_loader_utils.hpp"
#include "infrastructure/config/static_converter_config_provider.hpp"
#include "infrastructure/logging/console_diagnostics_sink.hpp"
#include "infrastructure/logging/console_logger.hpp"
#include "infrastructure/logging/file_error_report_writer.hpp"
#include "infrastructure/logging/validation_issue_reporter.hpp"
#include "infrastructure/persistence/importer/repository.hpp"
#include "infrastructure/persistence/importer/sqlite/connection.hpp"
#include "infrastructure/persistence/importer/sqlite/project_resolver.hpp"
#include "infrastructure/persistence/importer/sqlite/statement.hpp"
#include "infrastructure/persistence/importer/sqlite/writer.hpp"
#include "infrastructure/persistence/repositories/sqlite_project_repository.hpp"
#include "infrastructure/persistence/sqlite_data_query_service_internal.hpp"
#include "infrastructure/persistence/sqlite_database_health_checker.hpp"
#include "infrastructure/persistence/sqlite_time_sheet_repository.hpp"
#include "infrastructure/platform/android/android_platform_clock.hpp"
#include "infrastructure/platform/windows/windows_platform_clock.hpp"
#include "infrastructure/query/data/data_query_models.hpp"
#include "infrastructure/query/data/data_query_repository.hpp"
#include "infrastructure/query/data/data_query_types.hpp"
#include "infrastructure/query/data/internal/period.hpp"
#include "infrastructure/query/data/internal/report_mapping.hpp"
#include "infrastructure/query/data/internal/request.hpp"
#include "infrastructure/query/data/orchestrators/date_range_resolver.hpp"
#include "infrastructure/query/data/orchestrators/days_stats_orchestrator.hpp"
#include "infrastructure/query/data/orchestrators/list_query_orchestrator.hpp"
#include "infrastructure/query/data/orchestrators/orchestrators_boundary.hpp"
#include "infrastructure/query/data/orchestrators/report_chart_orchestrator.hpp"
#include "infrastructure/query/data/orchestrators/tree_orchestrator.hpp"
#include "infrastructure/query/data/renderers/data_query_renderer.hpp"
#include "infrastructure/query/data/renderers/renderers_boundary.hpp"
#include "infrastructure/query/data/renderers/semantic_json_renderer.hpp"
#include "infrastructure/query/data/renderers/text_renderer.hpp"
#include "infrastructure/query/data/stats/day_duration_stats_calculator.hpp"
#include "infrastructure/query/data/stats/report_chart_stats_calculator.hpp"
#include "infrastructure/query/data/stats/stats_boundary.hpp"
#include "infrastructure/query/data/stats/stats_models.hpp"
#include "infrastructure/reports/export_utils.hpp"
#include "infrastructure/reports/exporter.hpp"
#include "infrastructure/reports/lazy_sqlite_report_data_query_service.hpp"
#include "infrastructure/reports/lazy_sqlite_report_query_service.hpp"
#include "infrastructure/reports/report_dto_export_writer.hpp"
#include "infrastructure/reports/report_dto_formatter.hpp"
#include "infrastructure/reports/report_file_manager.hpp"
#include "infrastructure/reports/report_service.hpp"
#include "infrastructure/reports/services/daily_report_service.hpp"
#include "infrastructure/reports/services/monthly_report_service.hpp"
#include "infrastructure/reports/services/weekly_report_service.hpp"
#include "infrastructure/reports/services/yearly_report_service.hpp"
#include "infrastructure/reports/sqlite_report_data_query_service.hpp"

#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

class CompatPlatformClock final
    : public tracer_core::application::ports::IPlatformClock {
 public:
  [[nodiscard]] auto TodayLocalDateIso() const -> std::string override {
    return "2026-03-09";
  }

  [[nodiscard]] auto LocalUtcOffsetMinutes() const -> int override {
    return 8 * 60;
  }
};

inline auto Expect(bool condition, std::string_view message, int& failures)
    -> void {
  if (condition) {
    return;
  }
  ++failures;
  std::cerr << "[FAIL] " << message << '\n';
}

auto TestLegacyLoggingHeaders(int& failures) -> void;
auto TestLegacyPlatformHeaders(int& failures) -> void;
auto TestLegacyConfigHeaders(int& failures) -> void;
auto TestLegacyQueryDataStatsHeaders(int& failures) -> void;
auto TestLegacyQueryDataRepositoryAndRendererHeaders(int& failures) -> void;
auto TestLegacyQueryDataInternalHeaders(int& failures) -> void;
auto TestLegacyQueryDataOrchestratorHeaders(int& failures) -> void;
auto TestLegacyPersistenceWriteHeaders(int& failures) -> void;
auto TestLegacyPersistenceRuntimeHeaders(int& failures) -> void;
auto TestLegacyReportsExportHeaders(int& failures) -> void;
auto TestLegacyReportsDtoHeaders(int& failures) -> void;
auto TestLegacyReportsQueryingHeaders(int& failures) -> void;
auto TestLegacyReportsDataQueryingHeaders(int& failures) -> void;

#endif  // TRACER_CORE_TESTS_INFRASTRUCTURE_LEGACY_HEADERS_COMPAT_SUPPORT_HPP_
