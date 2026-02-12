// infrastructure/reports/shared/factories/generic_formatter_factory.hpp
#ifndef REPORTS_SHARED_FACTORIES_GENERIC_FORMATTER_FACTORY_H_
#define REPORTS_SHARED_FACTORIES_GENERIC_FORMATTER_FACTORY_H_

#include <filesystem>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "domain/reports/types/report_format.hpp"
#include "infrastructure/config/models/app_config.hpp"
#include "infrastructure/reports/shared/factories/dll_formatter_wrapper.hpp"
#include "infrastructure/reports/shared/factories/formatter_config_payload.hpp"
#include "infrastructure/reports/shared/interfaces/i_report_formatter.hpp"

namespace fs = std::filesystem;

template <typename ReportDataType>
class GenericFormatterFactory {
 public:
  using Creator =
      std::function<std::unique_ptr<IReportFormatter<ReportDataType>>(
          const AppConfig&)>;

  [[nodiscard]] static auto Create(ReportFormat format, const AppConfig& config)
      -> std::unique_ptr<IReportFormatter<ReportDataType>> {
    auto& creators = GetCreators();
    auto iter = creators.find(format);

    if (iter == creators.end()) {
      throw std::invalid_argument(
          "Unsupported report format or formatter not registered for this data "
          "type.");
    }

    return iter->second(config);
  }

  static void RegisterCreator(ReportFormat format, Creator creator) {
    GetCreators()[format] = std::move(creator);
  }

  static void RegisterDllFormatter(ReportFormat format,
                                   std::string dll_base_name) {
    RegisterCreator(
        format,
        [dll_base_name, format](const AppConfig& config)
            -> std::unique_ptr<IReportFormatter<ReportDataType>> {
          const uint32_t kConfigKind = GetFormatterConfigKind(format);
          if (kConfigKind == TT_FORMATTER_CONFIG_KIND_UNKNOWN) {
            throw std::invalid_argument(
                "Unsupported formatter config kind for selected report "
                "format.");
          }
          FormatterConfigPayload config_payload =
              BuildConfigPayloadFromLoaded(format, config);

          // Pass payload by const reference all the way through.
          // The ABI config view points to payload-owned string buffers, so a
          // move here may invalidate pointer addresses before DLL init.
          return LoadFromDll(dll_base_name, config, config_payload);
        });
  }

  static auto GetCreators() -> std::map<ReportFormat, Creator>& {
    static std::map<ReportFormat, Creator> creators;
    return creators;
  }

  [[nodiscard]] static auto GetFormatterConfigKind(ReportFormat format)
      -> uint32_t {
    if constexpr (std::is_same_v<ReportDataType, DailyReportData>) {
      switch (format) {
        case ReportFormat::kMarkdown:
          return TT_FORMATTER_CONFIG_KIND_DAY_MD;
        case ReportFormat::kLaTeX:
          return TT_FORMATTER_CONFIG_KIND_DAY_TEX;
        case ReportFormat::kTyp:
          return TT_FORMATTER_CONFIG_KIND_DAY_TYP;
      }
    }

    if constexpr (std::is_same_v<ReportDataType, MonthlyReportData>) {
      switch (format) {
        case ReportFormat::kMarkdown:
          return TT_FORMATTER_CONFIG_KIND_MONTH_MD;
        case ReportFormat::kLaTeX:
          return TT_FORMATTER_CONFIG_KIND_MONTH_TEX;
        case ReportFormat::kTyp:
          return TT_FORMATTER_CONFIG_KIND_MONTH_TYP;
      }
    }

    if constexpr ((std::is_same_v<ReportDataType, PeriodReportData>) ||
                  (std::is_same_v<ReportDataType, WeeklyReportData>) ||
                  (std::is_same_v<ReportDataType, YearlyReportData>) ||
                  (std::is_same_v<ReportDataType, RangeReportData>)) {
      switch (format) {
        case ReportFormat::kMarkdown:
          return TT_FORMATTER_CONFIG_KIND_RANGE_MD;
        case ReportFormat::kLaTeX:
          return TT_FORMATTER_CONFIG_KIND_RANGE_TEX;
        case ReportFormat::kTyp:
          return TT_FORMATTER_CONFIG_KIND_RANGE_TYP;
      }
    }

    return TT_FORMATTER_CONFIG_KIND_UNKNOWN;
  }

  [[nodiscard]] static auto BuildConfigPayloadFromLoaded(
      ReportFormat format, const AppConfig& config) -> FormatterConfigPayload {
    FormatterConfigPayload payload{};

    if constexpr (std::is_same_v<ReportDataType, DailyReportData>) {
      switch (format) {
        case ReportFormat::kMarkdown:
          payload.BuildFromLoadedDailyMdConfig(
              config.loaded_reports.markdown.day);
          return payload;
        case ReportFormat::kLaTeX:
          payload.BuildFromLoadedDailyTexConfig(
              config.loaded_reports.latex.day);
          return payload;
        case ReportFormat::kTyp:
          payload.BuildFromLoadedDailyTypConfig(
              config.loaded_reports.typst.day);
          return payload;
      }
    }

    if constexpr (std::is_same_v<ReportDataType, MonthlyReportData>) {
      switch (format) {
        case ReportFormat::kMarkdown:
          payload.BuildFromLoadedMonthMdConfig(
              config.loaded_reports.markdown.month);
          return payload;
        case ReportFormat::kLaTeX:
          payload.BuildFromLoadedMonthTexConfig(
              config.loaded_reports.latex.month);
          return payload;
        case ReportFormat::kTyp:
          payload.BuildFromLoadedMonthTypConfig(
              config.loaded_reports.typst.month);
          return payload;
      }
    }

    if constexpr (std::is_same_v<ReportDataType, PeriodReportData>) {
      switch (format) {
        case ReportFormat::kMarkdown:
          payload.BuildFromLoadedRangeMdConfig(
              config.loaded_reports.markdown.period.labels);
          return payload;
        case ReportFormat::kLaTeX:
          payload.BuildFromLoadedRangeTexConfig(
              config.loaded_reports.latex.period.labels,
              config.loaded_reports.latex.period.fonts,
              config.loaded_reports.latex.period.layout);
          return payload;
        case ReportFormat::kTyp:
          payload.BuildFromLoadedRangeTypConfig(
              config.loaded_reports.typst.period.labels,
              config.loaded_reports.typst.period.fonts,
              config.loaded_reports.typst.period.layout);
          return payload;
      }
    }

    if constexpr (std::is_same_v<ReportDataType, WeeklyReportData>) {
      switch (format) {
        case ReportFormat::kMarkdown:
          payload.BuildFromLoadedRangeMdConfig(
              config.loaded_reports.markdown.week.labels);
          return payload;
        case ReportFormat::kLaTeX:
          payload.BuildFromLoadedRangeTexConfig(
              config.loaded_reports.latex.week.labels,
              config.loaded_reports.latex.week.fonts,
              config.loaded_reports.latex.week.layout);
          return payload;
        case ReportFormat::kTyp:
          payload.BuildFromLoadedRangeTypConfig(
              config.loaded_reports.typst.week.labels,
              config.loaded_reports.typst.week.fonts,
              config.loaded_reports.typst.week.layout);
          return payload;
      }
    }

    if constexpr (std::is_same_v<ReportDataType, YearlyReportData>) {
      switch (format) {
        case ReportFormat::kMarkdown:
          payload.BuildFromLoadedRangeMdConfig(
              config.loaded_reports.markdown.year.labels);
          return payload;
        case ReportFormat::kLaTeX:
          payload.BuildFromLoadedRangeTexConfig(
              config.loaded_reports.latex.year.labels,
              config.loaded_reports.latex.year.fonts,
              config.loaded_reports.latex.year.layout);
          return payload;
        case ReportFormat::kTyp:
          payload.BuildFromLoadedRangeTypConfig(
              config.loaded_reports.typst.year.labels,
              config.loaded_reports.typst.year.fonts,
              config.loaded_reports.typst.year.layout);
          return payload;
      }
    }

    if constexpr (std::is_same_v<ReportDataType, RangeReportData>) {
      switch (format) {
        case ReportFormat::kMarkdown:
          payload.BuildFromLoadedRangeMdConfig(
              config.loaded_reports.markdown.period.labels);
          return payload;
        case ReportFormat::kLaTeX:
          payload.BuildFromLoadedRangeTexConfig(
              config.loaded_reports.latex.period.labels,
              config.loaded_reports.latex.period.fonts,
              config.loaded_reports.latex.period.layout);
          return payload;
        case ReportFormat::kTyp:
          payload.BuildFromLoadedRangeTypConfig(
              config.loaded_reports.typst.period.labels,
              config.loaded_reports.typst.period.fonts,
              config.loaded_reports.typst.period.layout);
          return payload;
      }
    }

    throw std::invalid_argument(
        "Unsupported report format or report data type for formatter payload.");
  }

  // Must take const reference, not by value:
  // moving FormatterConfigPayload can break C-view string pointer stability.
  [[nodiscard]] static auto LoadFromDll(
      const std::string& base_name, const AppConfig& config,
      const FormatterConfigPayload& config_payload)
      -> std::unique_ptr<IReportFormatter<ReportDataType>> {
    try {
      fs::path exe_dir(config.exe_dir_path);
      fs::path plugin_dir = exe_dir / "plugins";
      fs::path dll_path;

#ifdef _WIN32
      dll_path = plugin_dir / ("lib" + base_name + ".dll");
      if (!fs::exists(dll_path)) {
        dll_path = plugin_dir / (base_name + ".dll");
      }
#else
      dll_path = plugin_dir / ("lib" + base_name + ".so");
#endif
      if (!fs::exists(dll_path)) {
        throw std::runtime_error("Formatter plugin not found at: " +
                                 dll_path.string());
      }

      return std::make_unique<DllFormatterWrapper<ReportDataType>>(
          dll_path.string(), config_payload);

    } catch (const std::exception& exception) {
      std::cerr << "Error loading dynamic formatter: " << exception.what()
                << std::endl;
      throw;
    }
  }
};

#endif  // REPORTS_SHARED_FACTORIES_GENERIC_FORMATTER_FACTORY_H_
