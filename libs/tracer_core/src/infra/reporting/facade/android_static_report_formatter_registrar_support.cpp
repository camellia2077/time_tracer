#include "infra/reporting/facade/android_static_report_formatter_registrar_internal.hpp"

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

#include "infra/config/models/report_catalog.hpp"
#include "infra/reporting/shared/factories/generic_formatter_factory.hpp"
#include "infra/reporting/shared/interfaces/i_report_formatter.hpp"

namespace infrastructure::reports::detail {

auto BuildDayMarkdownCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<DailyReportData>>;
auto BuildMonthMarkdownCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<MonthlyReportData>>;
auto BuildPeriodMarkdownCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<PeriodReportData>>;
auto BuildWeeklyMarkdownCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<WeeklyReportData>>;
auto BuildYearlyMarkdownCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<YearlyReportData>>;

#if TT_REPORT_ENABLE_LATEX
auto BuildDayLatexCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<DailyReportData>>;
auto BuildMonthLatexCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<MonthlyReportData>>;
auto BuildPeriodLatexCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<PeriodReportData>>;
auto BuildWeeklyLatexCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<WeeklyReportData>>;
auto BuildYearlyLatexCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<YearlyReportData>>;
#endif

#if TT_REPORT_ENABLE_TYPST
auto BuildDayTypstCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<DailyReportData>>;
auto BuildMonthTypstCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<MonthlyReportData>>;
auto BuildPeriodTypstCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<PeriodReportData>>;
auto BuildWeeklyTypstCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<WeeklyReportData>>;
auto BuildYearlyTypstCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<YearlyReportData>>;
#endif

namespace {

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
        "Markdown formatter is disabled by Android static formatter policy.",
    .build_daily = &BuildDayMarkdownCoreFormatter,
    .build_monthly = &BuildMonthMarkdownCoreFormatter,
    .build_period = &BuildPeriodMarkdownCoreFormatter,
    .build_weekly = &BuildWeeklyMarkdownCoreFormatter,
    .build_yearly = &BuildYearlyMarkdownCoreFormatter};

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
    .build_yearly = &BuildYearlyLatexCoreFormatter};
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
    .build_yearly = &BuildYearlyTypstCoreFormatter};
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
