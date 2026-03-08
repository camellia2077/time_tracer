// infrastructure/reports/facade/android_static_report_formatter_registrar_support.cpp
#include "infrastructure/reports/facade/android_static_report_formatter_registrar_internal.hpp"

#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#ifndef TT_REPORT_ENABLE_LATEX
#define TT_REPORT_ENABLE_LATEX 1
#endif

#ifndef TT_REPORT_ENABLE_TYPST
#define TT_REPORT_ENABLE_TYPST 1
#endif

#include "infrastructure/config/models/report_catalog.hpp"
#include "infrastructure/reports/daily/formatters/markdown/day_md_formatter.hpp"
#include "infrastructure/reports/monthly/formatters/markdown/month_md_formatter.hpp"
#include "infrastructure/reports/range/formatters/markdown/range_md_formatter.hpp"
#include "infrastructure/reports/shared/factories/generic_formatter_factory.hpp"
#include "infrastructure/reports/shared/interfaces/i_report_formatter.hpp"

#if TT_REPORT_ENABLE_LATEX
#include "infrastructure/reports/daily/formatters/latex/day_tex_formatter.hpp"
#include "infrastructure/reports/monthly/formatters/latex/month_tex_formatter.hpp"
#include "infrastructure/reports/range/formatters/latex/range_tex_formatter.hpp"
#endif

#if TT_REPORT_ENABLE_TYPST
#include "infrastructure/reports/daily/formatters/typst/day_typ_formatter.hpp"
#include "infrastructure/reports/monthly/formatters/typst/month_typ_formatter.hpp"
#include "infrastructure/reports/range/formatters/typst/range_typ_formatter.hpp"
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

auto BuildDayMarkdownCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<DailyReportData>> {
  return BuildCoreFormatter<DayMdConfig, DayMdFormatter, DailyReportData>(
      catalog.loaded_reports.markdown.day);
}

auto BuildMonthMarkdownCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<MonthlyReportData>> {
  return BuildCoreFormatter<MonthMdConfig, MonthMdFormatter, MonthlyReportData>(
      catalog.loaded_reports.markdown.month);
}

#if TT_REPORT_ENABLE_LATEX
auto BuildDayLatexCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<DailyReportData>> {
  return BuildCoreFormatter<DayTexConfig, DayTexFormatter, DailyReportData>(
      catalog.loaded_reports.latex.day);
}

auto BuildMonthLatexCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<MonthlyReportData>> {
  return BuildCoreFormatter<MonthTexConfig, MonthTexFormatter,
                            MonthlyReportData>(
      catalog.loaded_reports.latex.month);
}
#endif

#if TT_REPORT_ENABLE_TYPST
auto BuildDayTypstCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<DailyReportData>> {
  return BuildCoreFormatter<DayTypConfig, DayTypFormatter, DailyReportData>(
      catalog.loaded_reports.typst.day);
}

auto BuildMonthTypstCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<MonthlyReportData>> {
  return BuildCoreFormatter<MonthTypConfig, MonthTypFormatter,
                            MonthlyReportData>(
      catalog.loaded_reports.typst.month);
}
#endif

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

auto BuildPeriodMarkdownCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<PeriodReportData>> {
  return std::make_unique<RangeReportFormatterAdapter<PeriodReportData>>(
      BuildRangeMarkdownCoreFormatter(
          catalog.loaded_reports.markdown.period.labels));
}

auto BuildWeeklyMarkdownCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<WeeklyReportData>> {
  return std::make_unique<RangeReportFormatterAdapter<WeeklyReportData>>(
      BuildRangeMarkdownCoreFormatter(catalog.loaded_reports.markdown.week.labels));
}

auto BuildYearlyMarkdownCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<YearlyReportData>> {
  return std::make_unique<RangeReportFormatterAdapter<YearlyReportData>>(
      BuildRangeMarkdownCoreFormatter(catalog.loaded_reports.markdown.year.labels));
}

#if TT_REPORT_ENABLE_LATEX
auto BuildPeriodLatexCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<PeriodReportData>> {
  return std::make_unique<RangeReportFormatterAdapter<PeriodReportData>>(
      BuildRangeLatexCoreFormatter(catalog.loaded_reports.latex.period.labels,
                                   catalog.loaded_reports.latex.period.fonts,
                                   catalog.loaded_reports.latex.period.layout));
}

auto BuildWeeklyLatexCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<WeeklyReportData>> {
  return std::make_unique<RangeReportFormatterAdapter<WeeklyReportData>>(
      BuildRangeLatexCoreFormatter(catalog.loaded_reports.latex.week.labels,
                                   catalog.loaded_reports.latex.week.fonts,
                                   catalog.loaded_reports.latex.week.layout));
}

auto BuildYearlyLatexCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<YearlyReportData>> {
  return std::make_unique<RangeReportFormatterAdapter<YearlyReportData>>(
      BuildRangeLatexCoreFormatter(catalog.loaded_reports.latex.year.labels,
                                   catalog.loaded_reports.latex.year.fonts,
                                   catalog.loaded_reports.latex.year.layout));
}
#endif

#if TT_REPORT_ENABLE_TYPST
auto BuildPeriodTypstCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<PeriodReportData>> {
  return std::make_unique<RangeReportFormatterAdapter<PeriodReportData>>(
      BuildRangeTypstCoreFormatter(catalog.loaded_reports.typst.period.labels,
                                   catalog.loaded_reports.typst.period.fonts,
                                   catalog.loaded_reports.typst.period.layout));
}

auto BuildWeeklyTypstCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<WeeklyReportData>> {
  return std::make_unique<RangeReportFormatterAdapter<WeeklyReportData>>(
      BuildRangeTypstCoreFormatter(catalog.loaded_reports.typst.week.labels,
                                   catalog.loaded_reports.typst.week.fonts,
                                   catalog.loaded_reports.typst.week.layout));
}

auto BuildYearlyTypstCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<YearlyReportData>> {
  return std::make_unique<RangeReportFormatterAdapter<YearlyReportData>>(
      BuildRangeTypstCoreFormatter(catalog.loaded_reports.typst.year.labels,
                                   catalog.loaded_reports.typst.year.fonts,
                                   catalog.loaded_reports.typst.year.layout));
}
#endif

template <typename ReportDataType, typename Builder>
auto RegisterCoreCreator(ReportFormat format, Builder&& builder) -> void {
  GenericFormatterFactory<ReportDataType>::RegisterCreator(
      format, std::forward<Builder>(builder));
}

template <typename ReportDataType>
auto RegisterDisabledCreator(ReportFormat format, std::string_view reason)
    -> void {
  GenericFormatterFactory<ReportDataType>::RegisterCreator(
      format,
      [reason](const ReportCatalog& /*catalog*/)
          -> std::unique_ptr<IReportFormatter<ReportDataType>> {
        throw std::invalid_argument(std::string(reason));
      });
}

auto RegisterDisabledFormatForAllTypes(ReportFormat format,
                                       std::string_view reason) -> void {
  RegisterDisabledCreator<DailyReportData>(format, reason);
  RegisterDisabledCreator<MonthlyReportData>(format, reason);
  RegisterDisabledCreator<PeriodReportData>(format, reason);
  RegisterDisabledCreator<WeeklyReportData>(format, reason);
  RegisterDisabledCreator<YearlyReportData>(format, reason);
}

using DailyBuilder = std::unique_ptr<IReportFormatter<DailyReportData>> (*)(
    const ReportCatalog&);
using MonthlyBuilder = std::unique_ptr<IReportFormatter<MonthlyReportData>> (*)(
    const ReportCatalog&);
using PeriodBuilder = std::unique_ptr<IReportFormatter<PeriodReportData>> (*)(
    const ReportCatalog&);
using WeeklyBuilder = std::unique_ptr<IReportFormatter<WeeklyReportData>> (*)(
    const ReportCatalog&);
using YearlyBuilder = std::unique_ptr<IReportFormatter<YearlyReportData>> (*)(
    const ReportCatalog&);
using AndroidPolicy = AndroidStaticReportFormatterPolicy;

struct FormatRegistrationRow {
  ReportFormat format;
  bool AndroidPolicy::* enabled_flag;
  std::string_view disabled_reason;
  DailyBuilder build_daily;
  MonthlyBuilder build_monthly;
  PeriodBuilder build_period;
  WeeklyBuilder build_weekly;
  YearlyBuilder build_yearly;
};

#if !TT_REPORT_ENABLE_LATEX
constexpr std::string_view kLatexCompiledOutReason =
    "LaTeX formatter is not compiled into this core build.";
#endif

#if !TT_REPORT_ENABLE_TYPST
constexpr std::string_view kTypstCompiledOutReason =
    "Typst formatter is not compiled into this core build.";
#endif

constexpr FormatRegistrationRow kMarkdownRegistrationRow = {
    .format = ReportFormat::kMarkdown,
    .enabled_flag = &AndroidPolicy::enable_markdown,
    .disabled_reason =
        "Markdown formatter is disabled by Android static "
        "formatter policy.",
    .build_daily = &BuildDayMarkdownCoreFormatter,
    .build_monthly = &BuildMonthMarkdownCoreFormatter,
    .build_period = &BuildPeriodMarkdownCoreFormatter,
    .build_weekly = &BuildWeeklyMarkdownCoreFormatter,
    .build_yearly = &BuildYearlyMarkdownCoreFormatter,
};

#if TT_REPORT_ENABLE_LATEX
constexpr FormatRegistrationRow kLatexRegistrationRow = {
    .format = ReportFormat::kLaTeX,
    .enabled_flag = &AndroidPolicy::enable_latex,
    .disabled_reason =
        "LaTeX formatter is disabled by Android static formatter policy.",
    .build_daily = &BuildDayLatexCoreFormatter,
    .build_monthly = &BuildMonthLatexCoreFormatter,
    .build_period = &BuildPeriodLatexCoreFormatter,
    .build_weekly = &BuildWeeklyLatexCoreFormatter,
    .build_yearly = &BuildYearlyLatexCoreFormatter,
};
#endif

#if TT_REPORT_ENABLE_TYPST
constexpr FormatRegistrationRow kTypstRegistrationRow = {
    .format = ReportFormat::kTyp,
    .enabled_flag = &AndroidPolicy::enable_typst,
    .disabled_reason =
        "Typst formatter is disabled by Android static formatter policy.",
    .build_daily = &BuildDayTypstCoreFormatter,
    .build_monthly = &BuildMonthTypstCoreFormatter,
    .build_period = &BuildPeriodTypstCoreFormatter,
    .build_weekly = &BuildWeeklyTypstCoreFormatter,
    .build_yearly = &BuildYearlyTypstCoreFormatter,
};
#endif

auto RegisterCreatorsForRow(const FormatRegistrationRow& row) -> void {
  RegisterCoreCreator<DailyReportData>(row.format, row.build_daily);
  RegisterCoreCreator<MonthlyReportData>(row.format, row.build_monthly);
  RegisterCoreCreator<PeriodReportData>(row.format, row.build_period);
  RegisterCoreCreator<WeeklyReportData>(row.format, row.build_weekly);
  RegisterCoreCreator<YearlyReportData>(row.format, row.build_yearly);
}

auto RegisterRowByPolicy(const FormatRegistrationRow& row, bool enabled)
    -> void {
  if (enabled) {
    RegisterCreatorsForRow(row);
    return;
  }
  RegisterDisabledFormatForAllTypes(row.format, row.disabled_reason);
}

}  // namespace

auto RegisterMarkdownFormatters(
    const AndroidStaticReportFormatterPolicy& policy) -> void {
  RegisterRowByPolicy(kMarkdownRegistrationRow, policy.enable_markdown);
}

auto RegisterLatexFormatters(const AndroidStaticReportFormatterPolicy& policy)
    -> void {
#if TT_REPORT_ENABLE_LATEX
  RegisterRowByPolicy(kLatexRegistrationRow, policy.enable_latex);
#else
  static_cast<void>(policy);
  RegisterDisabledFormatForAllTypes(ReportFormat::kLaTeX,
                                    kLatexCompiledOutReason);
#endif
}

auto RegisterTypstFormatters(const AndroidStaticReportFormatterPolicy& policy)
    -> void {
#if TT_REPORT_ENABLE_TYPST
  RegisterRowByPolicy(kTypstRegistrationRow, policy.enable_typst);
#else
  static_cast<void>(policy);
  RegisterDisabledFormatForAllTypes(ReportFormat::kTyp,
                                    kTypstCompiledOutReason);
#endif
}

}  // namespace infrastructure::reports::detail
