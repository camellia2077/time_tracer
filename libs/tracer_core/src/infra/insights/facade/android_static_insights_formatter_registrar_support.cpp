#include "infra/insights/facade/android_static_insights_formatter_registrar_internal.hpp"

#include <memory>
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
auto BuildDailyTypstCoreFormatter(const InsightsCatalog& catalog)
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

using DailyBuilder = std::unique_ptr<IInsightsFormatter<DailyInsightsData>> (*)(
    const InsightsCatalog&);
using MonthlyBuilder =
    std::unique_ptr<IInsightsFormatter<MonthlyInsightsData>> (*)(
        const InsightsCatalog&);
using PeriodBuilder =
    std::unique_ptr<IInsightsFormatter<PeriodInsightsData>> (*)(
        const InsightsCatalog&);
using WeeklyBuilder =
    std::unique_ptr<IInsightsFormatter<WeeklyInsightsData>> (*)(
        const InsightsCatalog&);
using YearlyBuilder =
    std::unique_ptr<IInsightsFormatter<YearlyInsightsData>> (*)(
        const InsightsCatalog&);

struct FormatRegistrationRow {
  InsightsFormat format;
  DailyBuilder build_daily;
  MonthlyBuilder build_monthly;
  PeriodBuilder build_period;
  WeeklyBuilder build_weekly;
  YearlyBuilder build_yearly;
};

constexpr FormatRegistrationRow kMarkdownRegistrationRow = {
    .format = InsightsFormat::kMarkdown,
    .build_daily = &BuildDayMarkdownCoreFormatter,
    .build_monthly = &BuildMonthMarkdownCoreFormatter,
    .build_period = &BuildPeriodMarkdownCoreFormatter,
    .build_weekly = &BuildWeeklyMarkdownCoreFormatter,
    .build_yearly = &BuildYearlyMarkdownCoreFormatter};

#if TT_INSIGHTS_ENABLE_LATEX
constexpr FormatRegistrationRow kLatexRegistrationRow = {
    .format = InsightsFormat::kLaTeX,
    .build_daily = &BuildDayLatexCoreFormatter,
    .build_monthly = &BuildMonthLatexCoreFormatter,
    .build_period = &BuildPeriodLatexCoreFormatter,
    .build_weekly = &BuildWeeklyLatexCoreFormatter,
    .build_yearly = &BuildYearlyLatexCoreFormatter};
#endif

#if TT_INSIGHTS_ENABLE_TYPST
constexpr FormatRegistrationRow kTypstRegistrationRow = {
    .format = InsightsFormat::kTyp,
    .build_daily = &BuildDailyTypstCoreFormatter,
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

auto UnregisterCreatorsForFormat(InsightsFormat format) -> void {
  GenericFormatterFactory<DailyInsightsData>::UnregisterCreator(format);
  GenericFormatterFactory<MonthlyInsightsData>::UnregisterCreator(format);
  GenericFormatterFactory<PeriodInsightsData>::UnregisterCreator(format);
  GenericFormatterFactory<WeeklyInsightsData>::UnregisterCreator(format);
  GenericFormatterFactory<YearlyInsightsData>::UnregisterCreator(format);
}

auto UnregisterCreatorsForRow(const FormatRegistrationRow& row) -> void {
  UnregisterCreatorsForFormat(row.format);
}

auto RegisterRowByPolicy(const FormatRegistrationRow& row, bool enabled)
    -> void {
  if (!enabled) {
    UnregisterCreatorsForRow(row);
    return;
  }
  RegisterCreatorsForRow(row);
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
  UnregisterCreatorsForFormat(InsightsFormat::kLaTeX);
#endif
}

auto RegisterTypstFormatters(const AndroidStaticInsightsFormatterPolicy& policy)
    -> void {
#if TT_INSIGHTS_ENABLE_TYPST
  RegisterRowByPolicy(kTypstRegistrationRow, policy.enable_typst);
#else
  static_cast<void>(policy);
  UnregisterCreatorsForFormat(InsightsFormat::kTyp);
#endif
}

}  // namespace infrastructure::insights::detail
