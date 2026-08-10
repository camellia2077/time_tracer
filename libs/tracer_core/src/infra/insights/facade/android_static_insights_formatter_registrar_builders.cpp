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
#include "infra/insights/daily/formatters/markdown/day_md_formatter.hpp"
#include "infra/insights/monthly/formatters/markdown/month_md_formatter.hpp"
#include "infra/insights/range/formatters/markdown/range_md_formatter.hpp"
#include "infra/insights/shared/factories/generic_formatter_factory.hpp"
#include "infra/insights/shared/interfaces/i_insights_formatter.hpp"

#if TT_INSIGHTS_ENABLE_LATEX
#include "infra/insights/daily/formatters/latex/day_tex_formatter.hpp"
#include "infra/insights/monthly/formatters/latex/month_tex_formatter.hpp"
#include "infra/insights/range/formatters/latex/range_tex_formatter.hpp"
#endif

#if TT_INSIGHTS_ENABLE_TYPST
#include "infra/insights/daily/formatters/typst/day_typ_formatter.hpp"
#include "infra/insights/monthly/formatters/typst/month_typ_formatter.hpp"
#include "infra/insights/range/formatters/typst/range_typ_formatter.hpp"
#endif

namespace infrastructure::insights::detail {
namespace {

template <typename FormatterConfigType, typename FormatterType,
          typename InsightsDataType, typename... ConfigArgs>
auto BuildCoreFormatter(ConfigArgs&&... config_args)
    -> std::unique_ptr<IInsightsFormatter<InsightsDataType>> {
  auto formatter_config = std::make_shared<FormatterConfigType>(
      std::forward<ConfigArgs>(config_args)...);
  return std::make_unique<FormatterType>(std::move(formatter_config));
}

template <typename InsightsDataType>
class RangeInsightsFormatterAdapter final
    : public IInsightsFormatter<InsightsDataType> {
 public:
  explicit RangeInsightsFormatterAdapter(
      std::unique_ptr<IInsightsFormatter<RangeInsightsData>> delegate)
      : delegate_(std::move(delegate)) {}

  [[nodiscard]] auto FormatInsights(const InsightsDataType& insights) const
      -> std::string override {
    return delegate_->FormatInsights(static_cast<const RangeInsightsData&>(insights));
  }

 private:
  std::unique_ptr<IInsightsFormatter<RangeInsightsData>> delegate_;
};

template <typename InsightsDataType>
auto BuildRangeAdapter(
    std::unique_ptr<IInsightsFormatter<RangeInsightsData>> delegate)
    -> std::unique_ptr<IInsightsFormatter<InsightsDataType>> {
  return std::make_unique<RangeInsightsFormatterAdapter<InsightsDataType>>(
      std::move(delegate));
}

auto BuildRangeMarkdownCoreFormatter(const RangeInsightsLabels& labels)
    -> std::unique_ptr<IInsightsFormatter<RangeInsightsData>> {
  return BuildCoreFormatter<RangeMdConfig, RangeMdFormatter, RangeInsightsData>(
      labels);
}

#if TT_INSIGHTS_ENABLE_LATEX
auto BuildRangeLatexCoreFormatter(const RangeInsightsLabels& labels,
                                  const FontConfig& fonts,
                                  const LayoutConfig& layout)
    -> std::unique_ptr<IInsightsFormatter<RangeInsightsData>> {
  return BuildCoreFormatter<RangeTexConfig, RangeTexFormatter, RangeInsightsData>(
      labels, fonts, layout);
}
#endif

#if TT_INSIGHTS_ENABLE_TYPST
auto BuildRangeTypstCoreFormatter(const RangeInsightsLabels& labels,
                                  const FontConfig& fonts,
                                  const LayoutConfig& layout)
    -> std::unique_ptr<IInsightsFormatter<RangeInsightsData>> {
  return BuildCoreFormatter<RangeTypConfig, RangeTypFormatter, RangeInsightsData>(
      labels, fonts, layout);
}
#endif

}  // namespace

auto BuildDayMarkdownCoreFormatter(const InsightsCatalog& catalog)
    -> std::unique_ptr<IInsightsFormatter<DailyInsightsData>> {
  return BuildCoreFormatter<DayMdConfig, DayMdFormatter, DailyInsightsData>(
      catalog.loaded_insights.markdown.day);
}

#if TT_INSIGHTS_ENABLE_LATEX
auto BuildDayLatexCoreFormatter(const InsightsCatalog& catalog)
    -> std::unique_ptr<IInsightsFormatter<DailyInsightsData>> {
  return BuildCoreFormatter<DayTexConfig, DayTexFormatter, DailyInsightsData>(
      catalog.loaded_insights.latex.day);
}
#endif

#if TT_INSIGHTS_ENABLE_TYPST
auto BuildDayTypstCoreFormatter(const InsightsCatalog& catalog)
    -> std::unique_ptr<IInsightsFormatter<DailyInsightsData>> {
  return BuildCoreFormatter<DayTypConfig, DayTypFormatter, DailyInsightsData>(
      catalog.loaded_insights.typst.day);
}
#endif

auto BuildMonthMarkdownCoreFormatter(const InsightsCatalog& catalog)
    -> std::unique_ptr<IInsightsFormatter<MonthlyInsightsData>> {
  return BuildRangeAdapter<MonthlyInsightsData>(BuildRangeMarkdownCoreFormatter(
      catalog.loaded_insights.markdown.month.labels));
}

#if TT_INSIGHTS_ENABLE_LATEX
auto BuildMonthLatexCoreFormatter(const InsightsCatalog& catalog)
    -> std::unique_ptr<IInsightsFormatter<MonthlyInsightsData>> {
  return BuildRangeAdapter<MonthlyInsightsData>(
      BuildRangeLatexCoreFormatter(catalog.loaded_insights.latex.month.labels,
                                   catalog.loaded_insights.latex.month.fonts,
                                   catalog.loaded_insights.latex.month.layout));
}
#endif

#if TT_INSIGHTS_ENABLE_TYPST
auto BuildMonthTypstCoreFormatter(const InsightsCatalog& catalog)
    -> std::unique_ptr<IInsightsFormatter<MonthlyInsightsData>> {
  return BuildRangeAdapter<MonthlyInsightsData>(
      BuildRangeTypstCoreFormatter(catalog.loaded_insights.typst.month.labels,
                                   catalog.loaded_insights.typst.month.fonts,
                                   catalog.loaded_insights.typst.month.layout));
}
#endif

auto BuildPeriodMarkdownCoreFormatter(const InsightsCatalog& catalog)
    -> std::unique_ptr<IInsightsFormatter<PeriodInsightsData>> {
  return BuildRangeAdapter<PeriodInsightsData>(BuildRangeMarkdownCoreFormatter(
      catalog.loaded_insights.markdown.period.labels));
}

auto BuildWeeklyMarkdownCoreFormatter(const InsightsCatalog& catalog)
    -> std::unique_ptr<IInsightsFormatter<WeeklyInsightsData>> {
  return BuildRangeAdapter<WeeklyInsightsData>(BuildRangeMarkdownCoreFormatter(
      catalog.loaded_insights.markdown.week.labels));
}

auto BuildYearlyMarkdownCoreFormatter(const InsightsCatalog& catalog)
    -> std::unique_ptr<IInsightsFormatter<YearlyInsightsData>> {
  return BuildRangeAdapter<YearlyInsightsData>(BuildRangeMarkdownCoreFormatter(
      catalog.loaded_insights.markdown.year.labels));
}

#if TT_INSIGHTS_ENABLE_LATEX
auto BuildPeriodLatexCoreFormatter(const InsightsCatalog& catalog)
    -> std::unique_ptr<IInsightsFormatter<PeriodInsightsData>> {
  return BuildRangeAdapter<PeriodInsightsData>(
      BuildRangeLatexCoreFormatter(catalog.loaded_insights.latex.period.labels,
                                   catalog.loaded_insights.latex.period.fonts,
                                   catalog.loaded_insights.latex.period.layout));
}

auto BuildWeeklyLatexCoreFormatter(const InsightsCatalog& catalog)
    -> std::unique_ptr<IInsightsFormatter<WeeklyInsightsData>> {
  return BuildRangeAdapter<WeeklyInsightsData>(
      BuildRangeLatexCoreFormatter(catalog.loaded_insights.latex.week.labels,
                                   catalog.loaded_insights.latex.week.fonts,
                                   catalog.loaded_insights.latex.week.layout));
}

auto BuildYearlyLatexCoreFormatter(const InsightsCatalog& catalog)
    -> std::unique_ptr<IInsightsFormatter<YearlyInsightsData>> {
  return BuildRangeAdapter<YearlyInsightsData>(
      BuildRangeLatexCoreFormatter(catalog.loaded_insights.latex.year.labels,
                                   catalog.loaded_insights.latex.year.fonts,
                                   catalog.loaded_insights.latex.year.layout));
}
#endif

#if TT_INSIGHTS_ENABLE_TYPST
auto BuildPeriodTypstCoreFormatter(const InsightsCatalog& catalog)
    -> std::unique_ptr<IInsightsFormatter<PeriodInsightsData>> {
  return BuildRangeAdapter<PeriodInsightsData>(
      BuildRangeTypstCoreFormatter(catalog.loaded_insights.typst.period.labels,
                                   catalog.loaded_insights.typst.period.fonts,
                                   catalog.loaded_insights.typst.period.layout));
}

auto BuildWeeklyTypstCoreFormatter(const InsightsCatalog& catalog)
    -> std::unique_ptr<IInsightsFormatter<WeeklyInsightsData>> {
  return BuildRangeAdapter<WeeklyInsightsData>(
      BuildRangeTypstCoreFormatter(catalog.loaded_insights.typst.week.labels,
                                   catalog.loaded_insights.typst.week.fonts,
                                   catalog.loaded_insights.typst.week.layout));
}

auto BuildYearlyTypstCoreFormatter(const InsightsCatalog& catalog)
    -> std::unique_ptr<IInsightsFormatter<YearlyInsightsData>> {
  return BuildRangeAdapter<YearlyInsightsData>(
      BuildRangeTypstCoreFormatter(catalog.loaded_insights.typst.year.labels,
                                   catalog.loaded_insights.typst.year.fonts,
                                   catalog.loaded_insights.typst.year.layout));
}
#endif

}  // namespace infrastructure::insights::detail
