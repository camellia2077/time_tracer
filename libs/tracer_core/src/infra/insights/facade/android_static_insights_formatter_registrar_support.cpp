#include "infra/insights/facade/android_static_insights_formatter_registrar_internal.hpp"

#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#ifndef TT_INSIGHTS_ENABLE_LATEX
#define TT_INSIGHTS_ENABLE_LATEX 1
#endif

#ifndef TT_INSIGHTS_ENABLE_TYPST
#define TT_INSIGHTS_ENABLE_TYPST 1
#endif

#include "infra/config/models/insights_catalog.hpp"
#include "infra/insights/shared/factories/generic_formatter_factory.hpp"
#include "infra/insights/shared/interfaces/i_insights_formatter.hpp"

namespace infrastructure::insights::detail {

auto BuildDayMarkdownCoreFormatter(const InsightsCatalog& catalog)
    -> std::unique_ptr<IInsightsFormatter<DailyInsightsData>>;
auto BuildMonthMarkdownCoreFormatter(const InsightsCatalog& catalog)
    -> std::unique_ptr<IInsightsFormatter<MonthlyInsightsData>>;
auto BuildPeriodMarkdownCoreFormatter(const InsightsCatalog& catalog)
    -> std::unique_ptr<IInsightsFormatter<PeriodInsightsData>>;
auto BuildWeeklyMarkdownCoreFormatter(const InsightsCatalog& catalog)
    -> std::unique_ptr<IInsightsFormatter<WeeklyInsightsData>>;
auto BuildYearlyMarkdownCoreFormatter(const InsightsCatalog& catalog)
    -> std::unique_ptr<IInsightsFormatter<YearlyInsightsData>>;

#if TT_INSIGHTS_ENABLE_LATEX
auto BuildDayLatexCoreFormatter(const InsightsCatalog& catalog)
    -> std::unique_ptr<IInsightsFormatter<DailyInsightsData>>;
auto BuildMonthLatexCoreFormatter(const InsightsCatalog& catalog)
    -> std::unique_ptr<IInsightsFormatter<MonthlyInsightsData>>;
auto BuildPeriodLatexCoreFormatter(const InsightsCatalog& catalog)
    -> std::unique_ptr<IInsightsFormatter<PeriodInsightsData>>;
auto BuildWeeklyLatexCoreFormatter(const InsightsCatalog& catalog)
    -> std::unique_ptr<IInsightsFormatter<WeeklyInsightsData>>;
auto BuildYearlyLatexCoreFormatter(const InsightsCatalog& catalog)
    -> std::unique_ptr<IInsightsFormatter<YearlyInsightsData>>;
#endif

#if TT_INSIGHTS_ENABLE_TYPST
auto BuildDayTypstCoreFormatter(const InsightsCatalog& catalog)
    -> std::unique_ptr<IInsightsFormatter<DailyInsightsData>>;
auto BuildMonthTypstCoreFormatter(const InsightsCatalog& catalog)
    -> std::unique_ptr<IInsightsFormatter<MonthlyInsightsData>>;
auto BuildPeriodTypstCoreFormatter(const InsightsCatalog& catalog)
    -> std::unique_ptr<IInsightsFormatter<PeriodInsightsData>>;
auto BuildWeeklyTypstCoreFormatter(const InsightsCatalog& catalog)
    -> std::unique_ptr<IInsightsFormatter<WeeklyInsightsData>>;
auto BuildYearlyTypstCoreFormatter(const InsightsCatalog& catalog)
    -> std::unique_ptr<IInsightsFormatter<YearlyInsightsData>>;
#endif

namespace {

template <typename InsightsDataType, typename Builder>
auto RegisterCoreCreator(InsightsFormat format, Builder&& builder) -> void {
  GenericFormatterFactory<InsightsDataType>::RegisterCreator(
      format, std::forward<Builder>(builder));
}

template <typename InsightsDataType>
auto RegisterDisabledCreator(InsightsFormat format, std::string_view reason)
    -> void {
  GenericFormatterFactory<InsightsDataType>::RegisterCreator(
      format,
      [reason](const InsightsCatalog& /*catalog*/)
          -> std::unique_ptr<IInsightsFormatter<InsightsDataType>> {
        throw std::invalid_argument(std::string(reason));
      });
}

auto RegisterDisabledFormatForAllTypes(InsightsFormat format,
                                       std::string_view reason) -> void {
  RegisterDisabledCreator<DailyInsightsData>(format, reason);
  RegisterDisabledCreator<MonthlyInsightsData>(format, reason);
  RegisterDisabledCreator<PeriodInsightsData>(format, reason);
  RegisterDisabledCreator<WeeklyInsightsData>(format, reason);
  RegisterDisabledCreator<YearlyInsightsData>(format, reason);
}

using DailyBuilder = std::unique_ptr<IInsightsFormatter<DailyInsightsData>> (*)(
    const InsightsCatalog&);
using MonthlyBuilder = std::unique_ptr<IInsightsFormatter<MonthlyInsightsData>> (*)(
    const InsightsCatalog&);
using PeriodBuilder = std::unique_ptr<IInsightsFormatter<PeriodInsightsData>> (*)(
    const InsightsCatalog&);
using WeeklyBuilder = std::unique_ptr<IInsightsFormatter<WeeklyInsightsData>> (*)(
    const InsightsCatalog&);
using YearlyBuilder = std::unique_ptr<IInsightsFormatter<YearlyInsightsData>> (*)(
    const InsightsCatalog&);
using AndroidPolicy = AndroidStaticInsightsFormatterPolicy;

struct FormatRegistrationRow {
  InsightsFormat format;
  bool AndroidPolicy::* enabled_flag;
  std::string_view disabled_reason;
  DailyBuilder build_daily;
  MonthlyBuilder build_monthly;
  PeriodBuilder build_period;
  WeeklyBuilder build_weekly;
  YearlyBuilder build_yearly;
};

#if !TT_INSIGHTS_ENABLE_LATEX
constexpr std::string_view kLatexCompiledOutReason =
    "LaTeX formatter is not compiled into this core build.";
#endif
#if !TT_INSIGHTS_ENABLE_TYPST
constexpr std::string_view kTypstCompiledOutReason =
    "Typst formatter is not compiled into this core build.";
#endif

constexpr FormatRegistrationRow kMarkdownRegistrationRow = {
    .format = InsightsFormat::kMarkdown,
    .enabled_flag = &AndroidPolicy::enable_markdown,
    .disabled_reason =
        "Markdown formatter is disabled by Android static formatter policy.",
    .build_daily = &BuildDayMarkdownCoreFormatter,
    .build_monthly = &BuildMonthMarkdownCoreFormatter,
    .build_period = &BuildPeriodMarkdownCoreFormatter,
    .build_weekly = &BuildWeeklyMarkdownCoreFormatter,
    .build_yearly = &BuildYearlyMarkdownCoreFormatter};

#if TT_INSIGHTS_ENABLE_LATEX
constexpr FormatRegistrationRow kLatexRegistrationRow = {
    .format = InsightsFormat::kLaTeX,
    .enabled_flag = &AndroidPolicy::enable_latex,
    .disabled_reason =
        "LaTeX formatter is disabled by Android static formatter policy.",
    .build_daily = &BuildDayLatexCoreFormatter,
    .build_monthly = &BuildMonthLatexCoreFormatter,
    .build_period = &BuildPeriodLatexCoreFormatter,
    .build_weekly = &BuildWeeklyLatexCoreFormatter,
    .build_yearly = &BuildYearlyLatexCoreFormatter};
#endif

#if TT_INSIGHTS_ENABLE_TYPST
constexpr FormatRegistrationRow kTypstRegistrationRow = {
    .format = InsightsFormat::kTyp,
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
  RegisterCoreCreator<DailyInsightsData>(row.format, row.build_daily);
  RegisterCoreCreator<MonthlyInsightsData>(row.format, row.build_monthly);
  RegisterCoreCreator<PeriodInsightsData>(row.format, row.build_period);
  RegisterCoreCreator<WeeklyInsightsData>(row.format, row.build_weekly);
  RegisterCoreCreator<YearlyInsightsData>(row.format, row.build_yearly);
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
    const AndroidStaticInsightsFormatterPolicy& policy) -> void {
  RegisterRowByPolicy(kMarkdownRegistrationRow, policy.enable_markdown);
}

auto RegisterLatexFormatters(const AndroidStaticInsightsFormatterPolicy& policy)
    -> void {
#if TT_INSIGHTS_ENABLE_LATEX
  RegisterRowByPolicy(kLatexRegistrationRow, policy.enable_latex);
#else
  static_cast<void>(policy);
  RegisterDisabledFormatForAllTypes(InsightsFormat::kLaTeX,
                                    kLatexCompiledOutReason);
#endif
}

auto RegisterTypstFormatters(const AndroidStaticInsightsFormatterPolicy& policy)
    -> void {
#if TT_INSIGHTS_ENABLE_TYPST
  RegisterRowByPolicy(kTypstRegistrationRow, policy.enable_typst);
#else
  static_cast<void>(policy);
  RegisterDisabledFormatForAllTypes(InsightsFormat::kTyp,
                                    kTypstCompiledOutReason);
#endif
}

}  // namespace infrastructure::insights::detail
