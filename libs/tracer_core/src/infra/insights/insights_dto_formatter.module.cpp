#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "infra/insights/insights_dto_formatter.hpp"
#include "application/ports/insights/i_insights_dto_formatter.hpp"
#include "infra/config/models/insights_catalog.hpp"
#include "infra/insights/shared/factories/generic_formatter_factory.hpp"
#include "infra/insights/shared/interfaces/i_insights_formatter.hpp"

namespace tracer::core::infrastructure::insights {

InsightsDtoFormatter::InsightsDtoFormatter(
    const InsightsCatalog& insights_catalog)
    : insights_catalog_(insights_catalog) {}

auto InsightsDtoFormatter::FormatDaily(const DailyInsightsData& insights,
                                       InsightsFormat format) -> std::string {
  return FormatWithCache(insights, format, daily_cache_);
}

auto InsightsDtoFormatter::FormatMonthly(const MonthlyInsightsData& insights,
                                         InsightsFormat format) -> std::string {
  return FormatWithCache(insights, format, monthly_cache_);
}

auto InsightsDtoFormatter::FormatPeriod(const PeriodInsightsData& insights,
                                        InsightsFormat format) -> std::string {
  return FormatWithCache(insights, format, period_cache_);
}

auto InsightsDtoFormatter::FormatWeekly(const WeeklyInsightsData& insights,
                                        InsightsFormat format) -> std::string {
  return FormatWithCache(insights, format, weekly_cache_);
}

auto InsightsDtoFormatter::FormatYearly(const YearlyInsightsData& insights,
                                        InsightsFormat format) -> std::string {
  return FormatWithCache(insights, format, yearly_cache_);
}

template <typename InsightsDataType>
auto FormatLocalizedInsights(const InsightsDataType& insights,
                             InsightsFormat format,
                             const InsightsCatalog& base_catalog,
                             const MarkdownInsightsConfigs* localized_markdown)
    -> std::string {
  if (localized_markdown == nullptr || format != InsightsFormat::kMarkdown) {
    return GenericFormatterFactory<InsightsDataType>::Create(format,
                                                             base_catalog)
        ->FormatInsights(insights);
  }
  InsightsCatalog localized_catalog = base_catalog;
  localized_catalog.loaded_insights.markdown = *localized_markdown;
  return GenericFormatterFactory<InsightsDataType>::Create(format,
                                                           localized_catalog)
      ->FormatInsights(insights);
}

auto FindLocalizedMarkdown(const InsightsCatalog& catalog,
                           std::string_view locale)
    -> const MarkdownInsightsConfigs* {
  const auto iter =
      catalog.loaded_insights.markdown_locales.find(std::string(locale));
  return iter == catalog.loaded_insights.markdown_locales.end() ? nullptr
                                                                : &iter->second;
}

auto InsightsDtoFormatter::FormatDailyLocalized(
    const DailyInsightsData& insights, InsightsFormat format,
    std::string_view locale) -> std::string {
  return FormatLocalizedInsights(
      insights, format, insights_catalog_,
      FindLocalizedMarkdown(insights_catalog_, locale));
}

auto InsightsDtoFormatter::FormatMonthlyLocalized(
    const MonthlyInsightsData& insights, InsightsFormat format,
    std::string_view locale) -> std::string {
  return FormatLocalizedInsights(
      insights, format, insights_catalog_,
      FindLocalizedMarkdown(insights_catalog_, locale));
}

auto InsightsDtoFormatter::FormatPeriodLocalized(
    const PeriodInsightsData& insights, InsightsFormat format,
    std::string_view locale) -> std::string {
  return FormatLocalizedInsights(
      insights, format, insights_catalog_,
      FindLocalizedMarkdown(insights_catalog_, locale));
}

auto InsightsDtoFormatter::FormatWeeklyLocalized(
    const WeeklyInsightsData& insights, InsightsFormat format,
    std::string_view locale) -> std::string {
  return FormatLocalizedInsights(
      insights, format, insights_catalog_,
      FindLocalizedMarkdown(insights_catalog_, locale));
}

auto InsightsDtoFormatter::FormatYearlyLocalized(
    const YearlyInsightsData& insights, InsightsFormat format,
    std::string_view locale) -> std::string {
  return FormatLocalizedInsights(
      insights, format, insights_catalog_,
      FindLocalizedMarkdown(insights_catalog_, locale));
}

template <typename InsightsDataType>
auto InsightsDtoFormatter::FormatWithCache(
    const InsightsDataType& insights, InsightsFormat format,
    std::map<InsightsFormat,
             std::unique_ptr<IInsightsFormatter<InsightsDataType>>>& cache)
    -> std::string {
  auto formatter_iter = cache.find(format);
  if (formatter_iter == cache.end()) {
    auto formatter = GenericFormatterFactory<InsightsDataType>::Create(
        format, insights_catalog_);
    formatter_iter = cache.emplace(format, std::move(formatter)).first;
  }
  return formatter_iter->second->FormatInsights(insights);
}

}  // namespace tracer::core::infrastructure::insights
