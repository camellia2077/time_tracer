#include "infra/reporting/facade/android_static_report_formatter_registrar_internal.hpp"

#include <memory>
#include <utility>

#ifndef TT_REPORT_ENABLE_LATEX
#define TT_REPORT_ENABLE_LATEX 1
#endif

#ifndef TT_REPORT_ENABLE_TYPST
#define TT_REPORT_ENABLE_TYPST 1
#endif

#include "infra/config/models/report_catalog.hpp"
#include "infra/reporting/daily/formatters/markdown/day_md_formatter.hpp"
#include "infra/reporting/monthly/formatters/markdown/month_md_formatter.hpp"
#include "infra/reporting/range/formatters/markdown/range_md_formatter.hpp"
#include "infra/reporting/shared/factories/generic_formatter_factory.hpp"
#include "infra/reporting/shared/interfaces/i_report_formatter.hpp"

#if TT_REPORT_ENABLE_LATEX
#include "infra/reporting/daily/formatters/latex/day_tex_formatter.hpp"
#include "infra/reporting/monthly/formatters/latex/month_tex_formatter.hpp"
#include "infra/reporting/range/formatters/latex/range_tex_formatter.hpp"
#endif

#if TT_REPORT_ENABLE_TYPST
#include "infra/reporting/daily/formatters/typst/day_typ_formatter.hpp"
#include "infra/reporting/monthly/formatters/typst/month_typ_formatter.hpp"
#include "infra/reporting/range/formatters/typst/range_typ_formatter.hpp"
#endif

namespace infrastructure::reports::detail {
namespace {

template <typename FormatterConfigType, typename FormatterType,
          typename ReportDataType, typename... ConfigArgs>
auto BuildCoreFormatter(ConfigArgs&&... config_args)
    -> std::unique_ptr<IReportFormatter<ReportDataType>> {
  auto formatter_config = std::make_shared<FormatterConfigType>(
      std::forward<ConfigArgs>(config_args)...);
  return std::make_unique<FormatterType>(std::move(formatter_config));
}

template <typename ReportDataType>
class RangeReportFormatterAdapter final
    : public IReportFormatter<ReportDataType> {
 public:
  explicit RangeReportFormatterAdapter(
      std::unique_ptr<IReportFormatter<RangeReportData>> delegate)
      : delegate_(std::move(delegate)) {}

  [[nodiscard]] auto FormatReport(const ReportDataType& report) const
      -> std::string override {
    return delegate_->FormatReport(static_cast<const RangeReportData&>(report));
  }

 private:
  std::unique_ptr<IReportFormatter<RangeReportData>> delegate_;
};

template <typename ReportDataType>
auto BuildRangeAdapter(
    std::unique_ptr<IReportFormatter<RangeReportData>> delegate)
    -> std::unique_ptr<IReportFormatter<ReportDataType>> {
  return std::make_unique<RangeReportFormatterAdapter<ReportDataType>>(
      std::move(delegate));
}

auto BuildRangeMarkdownCoreFormatter(const RangeReportLabels& labels)
    -> std::unique_ptr<IReportFormatter<RangeReportData>> {
  return BuildCoreFormatter<RangeMdConfig, RangeMdFormatter, RangeReportData>(
      labels);
}

#if TT_REPORT_ENABLE_LATEX
auto BuildRangeLatexCoreFormatter(const RangeReportLabels& labels,
                                  const FontConfig& fonts,
                                  const LayoutConfig& layout)
    -> std::unique_ptr<IReportFormatter<RangeReportData>> {
  return BuildCoreFormatter<RangeTexConfig, RangeTexFormatter, RangeReportData>(
      labels, fonts, layout);
}
#endif

#if TT_REPORT_ENABLE_TYPST
auto BuildRangeTypstCoreFormatter(const RangeReportLabels& labels,
                                  const FontConfig& fonts,
                                  const LayoutConfig& layout)
    -> std::unique_ptr<IReportFormatter<RangeReportData>> {
  return BuildCoreFormatter<RangeTypConfig, RangeTypFormatter, RangeReportData>(
      labels, fonts, layout);
}
#endif

}  // namespace

auto BuildDayMarkdownCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<DailyReportData>> {
  return BuildCoreFormatter<DayMdConfig, DayMdFormatter, DailyReportData>(
      catalog.loaded_reports.markdown.day);
}

#if TT_REPORT_ENABLE_LATEX
auto BuildDayLatexCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<DailyReportData>> {
  return BuildCoreFormatter<DayTexConfig, DayTexFormatter, DailyReportData>(
      catalog.loaded_reports.latex.day);
}
#endif

#if TT_REPORT_ENABLE_TYPST
auto BuildDayTypstCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<DailyReportData>> {
  return BuildCoreFormatter<DayTypConfig, DayTypFormatter, DailyReportData>(
      catalog.loaded_reports.typst.day);
}
#endif

auto BuildMonthMarkdownCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<MonthlyReportData>> {
  return BuildRangeAdapter<MonthlyReportData>(BuildRangeMarkdownCoreFormatter(
      catalog.loaded_reports.markdown.month.labels));
}

#if TT_REPORT_ENABLE_LATEX
auto BuildMonthLatexCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<MonthlyReportData>> {
  return BuildRangeAdapter<MonthlyReportData>(
      BuildRangeLatexCoreFormatter(catalog.loaded_reports.latex.month.labels,
                                   catalog.loaded_reports.latex.month.fonts,
                                   catalog.loaded_reports.latex.month.layout));
}
#endif

#if TT_REPORT_ENABLE_TYPST
auto BuildMonthTypstCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<MonthlyReportData>> {
  return BuildRangeAdapter<MonthlyReportData>(
      BuildRangeTypstCoreFormatter(catalog.loaded_reports.typst.month.labels,
                                   catalog.loaded_reports.typst.month.fonts,
                                   catalog.loaded_reports.typst.month.layout));
}
#endif

auto BuildPeriodMarkdownCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<PeriodReportData>> {
  return BuildRangeAdapter<PeriodReportData>(BuildRangeMarkdownCoreFormatter(
      catalog.loaded_reports.markdown.period.labels));
}

auto BuildWeeklyMarkdownCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<WeeklyReportData>> {
  return BuildRangeAdapter<WeeklyReportData>(BuildRangeMarkdownCoreFormatter(
      catalog.loaded_reports.markdown.week.labels));
}

auto BuildYearlyMarkdownCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<YearlyReportData>> {
  return BuildRangeAdapter<YearlyReportData>(BuildRangeMarkdownCoreFormatter(
      catalog.loaded_reports.markdown.year.labels));
}

#if TT_REPORT_ENABLE_LATEX
auto BuildPeriodLatexCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<PeriodReportData>> {
  return BuildRangeAdapter<PeriodReportData>(
      BuildRangeLatexCoreFormatter(catalog.loaded_reports.latex.period.labels,
                                   catalog.loaded_reports.latex.period.fonts,
                                   catalog.loaded_reports.latex.period.layout));
}

auto BuildWeeklyLatexCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<WeeklyReportData>> {
  return BuildRangeAdapter<WeeklyReportData>(
      BuildRangeLatexCoreFormatter(catalog.loaded_reports.latex.week.labels,
                                   catalog.loaded_reports.latex.week.fonts,
                                   catalog.loaded_reports.latex.week.layout));
}

auto BuildYearlyLatexCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<YearlyReportData>> {
  return BuildRangeAdapter<YearlyReportData>(
      BuildRangeLatexCoreFormatter(catalog.loaded_reports.latex.year.labels,
                                   catalog.loaded_reports.latex.year.fonts,
                                   catalog.loaded_reports.latex.year.layout));
}
#endif

#if TT_REPORT_ENABLE_TYPST
auto BuildPeriodTypstCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<PeriodReportData>> {
  return BuildRangeAdapter<PeriodReportData>(
      BuildRangeTypstCoreFormatter(catalog.loaded_reports.typst.period.labels,
                                   catalog.loaded_reports.typst.period.fonts,
                                   catalog.loaded_reports.typst.period.layout));
}

auto BuildWeeklyTypstCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<WeeklyReportData>> {
  return BuildRangeAdapter<WeeklyReportData>(
      BuildRangeTypstCoreFormatter(catalog.loaded_reports.typst.week.labels,
                                   catalog.loaded_reports.typst.week.fonts,
                                   catalog.loaded_reports.typst.week.layout));
}

auto BuildYearlyTypstCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<YearlyReportData>> {
  return BuildRangeAdapter<YearlyReportData>(
      BuildRangeTypstCoreFormatter(catalog.loaded_reports.typst.year.labels,
                                   catalog.loaded_reports.typst.year.fonts,
                                   catalog.loaded_reports.typst.year.layout));
}
#endif

}  // namespace infrastructure::reports::detail
