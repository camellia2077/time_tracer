// infrastructure/reports/shared/factories/generic_formatter_factory_resolver.cpp
#include <stdexcept>

#include "infrastructure/reports/shared/factories/generic_formatter_factory.hpp"

namespace generic_formatter_factory_detail {

[[nodiscard]] auto ResolveFormatterConfigKind(ReportDataKind report_data_kind,
                                              ReportFormat format) -> uint32_t {
  switch (report_data_kind) {
    case ReportDataKind::kDaily:
      switch (format) {
        case ReportFormat::kMarkdown:
          return TT_FORMATTER_CONFIG_KIND_DAY_MD;
        case ReportFormat::kLaTeX:
          return TT_FORMATTER_CONFIG_KIND_DAY_TEX;
        case ReportFormat::kTyp:
          return TT_FORMATTER_CONFIG_KIND_DAY_TYP;
      }
      break;
    case ReportDataKind::kMonthly:
      switch (format) {
        case ReportFormat::kMarkdown:
          return TT_FORMATTER_CONFIG_KIND_MONTH_MD;
        case ReportFormat::kLaTeX:
          return TT_FORMATTER_CONFIG_KIND_MONTH_TEX;
        case ReportFormat::kTyp:
          return TT_FORMATTER_CONFIG_KIND_MONTH_TYP;
      }
      break;
    case ReportDataKind::kPeriod:
    case ReportDataKind::kWeekly:
    case ReportDataKind::kYearly:
    case ReportDataKind::kRange:
      switch (format) {
        case ReportFormat::kMarkdown:
          return TT_FORMATTER_CONFIG_KIND_RANGE_MD;
        case ReportFormat::kLaTeX:
          return TT_FORMATTER_CONFIG_KIND_RANGE_TEX;
        case ReportFormat::kTyp:
          return TT_FORMATTER_CONFIG_KIND_RANGE_TYP;
      }
      break;
  }

  return TT_FORMATTER_CONFIG_KIND_UNKNOWN;
}

[[nodiscard]] auto BuildConfigPayloadFromLoaded(ReportDataKind report_data_kind,
                                                ReportFormat format,
                                                const ReportCatalog& catalog)
    -> FormatterConfigPayload {
  FormatterConfigPayload payload{};

  switch (report_data_kind) {
    case ReportDataKind::kDaily:
      switch (format) {
        case ReportFormat::kMarkdown:
          payload.BuildFromLoadedDailyMdConfig(
              catalog.loaded_reports.markdown.day);
          return payload;
        case ReportFormat::kLaTeX:
          payload.BuildFromLoadedDailyTexConfig(
              catalog.loaded_reports.latex.day);
          return payload;
        case ReportFormat::kTyp:
          payload.BuildFromLoadedDailyTypConfig(
              catalog.loaded_reports.typst.day);
          return payload;
      }
      break;
    case ReportDataKind::kMonthly:
      switch (format) {
        case ReportFormat::kMarkdown:
          payload.BuildFromLoadedMonthMdConfig(
              catalog.loaded_reports.markdown.month);
          return payload;
        case ReportFormat::kLaTeX:
          payload.BuildFromLoadedMonthTexConfig(
              catalog.loaded_reports.latex.month);
          return payload;
        case ReportFormat::kTyp:
          payload.BuildFromLoadedMonthTypConfig(
              catalog.loaded_reports.typst.month);
          return payload;
      }
      break;
    case ReportDataKind::kPeriod:
      switch (format) {
        case ReportFormat::kMarkdown:
          payload.BuildFromLoadedRangeMdConfig(
              catalog.loaded_reports.markdown.period.labels);
          return payload;
        case ReportFormat::kLaTeX:
          payload.BuildFromLoadedRangeTexConfig(
              catalog.loaded_reports.latex.period.labels,
              catalog.loaded_reports.latex.period.fonts,
              catalog.loaded_reports.latex.period.layout);
          return payload;
        case ReportFormat::kTyp:
          payload.BuildFromLoadedRangeTypConfig(
              catalog.loaded_reports.typst.period.labels,
              catalog.loaded_reports.typst.period.fonts,
              catalog.loaded_reports.typst.period.layout);
          return payload;
      }
      break;
    case ReportDataKind::kWeekly:
      switch (format) {
        case ReportFormat::kMarkdown:
          payload.BuildFromLoadedRangeMdConfig(
              catalog.loaded_reports.markdown.week.labels);
          return payload;
        case ReportFormat::kLaTeX:
          payload.BuildFromLoadedRangeTexConfig(
              catalog.loaded_reports.latex.week.labels,
              catalog.loaded_reports.latex.week.fonts,
              catalog.loaded_reports.latex.week.layout);
          return payload;
        case ReportFormat::kTyp:
          payload.BuildFromLoadedRangeTypConfig(
              catalog.loaded_reports.typst.week.labels,
              catalog.loaded_reports.typst.week.fonts,
              catalog.loaded_reports.typst.week.layout);
          return payload;
      }
      break;
    case ReportDataKind::kYearly:
      switch (format) {
        case ReportFormat::kMarkdown:
          payload.BuildFromLoadedRangeMdConfig(
              catalog.loaded_reports.markdown.year.labels);
          return payload;
        case ReportFormat::kLaTeX:
          payload.BuildFromLoadedRangeTexConfig(
              catalog.loaded_reports.latex.year.labels,
              catalog.loaded_reports.latex.year.fonts,
              catalog.loaded_reports.latex.year.layout);
          return payload;
        case ReportFormat::kTyp:
          payload.BuildFromLoadedRangeTypConfig(
              catalog.loaded_reports.typst.year.labels,
              catalog.loaded_reports.typst.year.fonts,
              catalog.loaded_reports.typst.year.layout);
          return payload;
      }
      break;
    case ReportDataKind::kRange:
      switch (format) {
        case ReportFormat::kMarkdown:
          payload.BuildFromLoadedRangeMdConfig(
              catalog.loaded_reports.markdown.period.labels);
          return payload;
        case ReportFormat::kLaTeX:
          payload.BuildFromLoadedRangeTexConfig(
              catalog.loaded_reports.latex.period.labels,
              catalog.loaded_reports.latex.period.fonts,
              catalog.loaded_reports.latex.period.layout);
          return payload;
        case ReportFormat::kTyp:
          payload.BuildFromLoadedRangeTypConfig(
              catalog.loaded_reports.typst.period.labels,
              catalog.loaded_reports.typst.period.fonts,
              catalog.loaded_reports.typst.period.layout);
          return payload;
      }
      break;
  }

  throw std::invalid_argument(
      "Unsupported report format or report data type for formatter payload.");
}

}  // namespace generic_formatter_factory_detail
