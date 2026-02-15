// infrastructure/reports/facade/android_static_report_formatter_registrar.cpp
#include "infrastructure/reports/facade/android_static_report_formatter_registrar.hpp"

#include <array>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "infrastructure/config/models/report_catalog.hpp"
#include "infrastructure/reports/daily/formatters/latex/day_tex_formatter.hpp"
#include "infrastructure/reports/daily/formatters/markdown/day_md_formatter.hpp"
#include "infrastructure/reports/daily/formatters/typst/day_typ_formatter.hpp"
#include "infrastructure/reports/monthly/formatters/latex/month_tex_formatter.hpp"
#include "infrastructure/reports/monthly/formatters/markdown/month_md_formatter.hpp"
#include "infrastructure/reports/monthly/formatters/typst/month_typ_formatter.hpp"
#include "infrastructure/reports/range/formatters/latex/range_tex_formatter.hpp"
#include "infrastructure/reports/range/formatters/markdown/range_md_formatter.hpp"
#include "infrastructure/reports/range/formatters/typst/range_typ_formatter.hpp"
#include "infrastructure/reports/shared/factories/formatter_config_payload.hpp"
#include "infrastructure/reports/shared/factories/generic_formatter_factory.hpp"
#include "infrastructure/reports/shared/interfaces/i_report_formatter.hpp"

namespace {

template <typename PayloadType, typename FormatterConfigType,
          typename FormatterType, typename ReportDataType>
auto BuildCoreFormatter(const TtFormatterConfig& config_view,
                        uint32_t expected_kind, const char* format_name)
    -> std::unique_ptr<IReportFormatter<ReportDataType>> {
  if (config_view.configData == nullptr) {
    throw std::runtime_error(std::string("Missing formatter config data for ") +
                             format_name + ".");
  }
  if (config_view.configKind != expected_kind) {
    throw std::runtime_error(
        std::string("Unexpected formatter config kind for ") + format_name +
        ".");
  }
  if (config_view.configDataSize != sizeof(PayloadType)) {
    throw std::runtime_error(std::string("Invalid formatter config size for ") +
                             format_name + ".");
  }

  const auto* payload = static_cast<const PayloadType*>(config_view.configData);
  auto formatter_config = std::make_shared<FormatterConfigType>(*payload);
  return std::make_unique<FormatterType>(std::move(formatter_config));
}

auto BuildDayMarkdownCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<DailyReportData>> {
  FormatterConfigPayload payload;
  payload.BuildFromLoadedDailyMdConfig(catalog.loaded_reports.markdown.day);
  return BuildCoreFormatter<TtDayMdConfigV1, DayMdConfig, DayMdFormatter,
                            DailyReportData>(
      payload.GetCConfig(), TT_FORMATTER_CONFIG_KIND_DAY_MD, "day markdown");
}

auto BuildMonthMarkdownCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<MonthlyReportData>> {
  FormatterConfigPayload payload;
  payload.BuildFromLoadedMonthMdConfig(catalog.loaded_reports.markdown.month);
  return BuildCoreFormatter<TtMonthMdConfigV1, MonthMdConfig, MonthMdFormatter,
                            MonthlyReportData>(
      payload.GetCConfig(), TT_FORMATTER_CONFIG_KIND_MONTH_MD,
      "month markdown");
}

auto BuildDayLatexCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<DailyReportData>> {
  FormatterConfigPayload payload;
  payload.BuildFromLoadedDailyTexConfig(catalog.loaded_reports.latex.day);
  return BuildCoreFormatter<TtDayTexConfigV1, DayTexConfig, DayTexFormatter,
                            DailyReportData>(
      payload.GetCConfig(), TT_FORMATTER_CONFIG_KIND_DAY_TEX, "day latex");
}

auto BuildMonthLatexCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<MonthlyReportData>> {
  FormatterConfigPayload payload;
  payload.BuildFromLoadedMonthTexConfig(catalog.loaded_reports.latex.month);
  return BuildCoreFormatter<TtMonthTexConfigV1, MonthTexConfig,
                            MonthTexFormatter, MonthlyReportData>(
      payload.GetCConfig(), TT_FORMATTER_CONFIG_KIND_MONTH_TEX, "month latex");
}

auto BuildDayTypstCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<DailyReportData>> {
  FormatterConfigPayload payload;
  payload.BuildFromLoadedDailyTypConfig(catalog.loaded_reports.typst.day);
  return BuildCoreFormatter<TtDayTypConfigV1, DayTypConfig, DayTypFormatter,
                            DailyReportData>(
      payload.GetCConfig(), TT_FORMATTER_CONFIG_KIND_DAY_TYP, "day typst");
}

auto BuildMonthTypstCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<MonthlyReportData>> {
  FormatterConfigPayload payload;
  payload.BuildFromLoadedMonthTypConfig(catalog.loaded_reports.typst.month);
  return BuildCoreFormatter<TtMonthTypConfigV1, MonthTypConfig,
                            MonthTypFormatter, MonthlyReportData>(
      payload.GetCConfig(), TT_FORMATTER_CONFIG_KIND_MONTH_TYP, "month typst");
}

auto BuildRangeMarkdownCoreFormatter(const RangeReportLabels& labels)
    -> std::unique_ptr<IReportFormatter<RangeReportData>> {
  FormatterConfigPayload payload;
  payload.BuildFromLoadedRangeMdConfig(labels);
  return BuildCoreFormatter<TtRangeMdConfigV1, RangeMdConfig, RangeMdFormatter,
                            RangeReportData>(payload.GetCConfig(),
                                             TT_FORMATTER_CONFIG_KIND_RANGE_MD,
                                             "range markdown");
}

auto BuildRangeLatexCoreFormatter(const RangeReportLabels& labels,
                                  const FontConfig& fonts,
                                  const LayoutConfig& layout)
    -> std::unique_ptr<IReportFormatter<RangeReportData>> {
  FormatterConfigPayload payload;
  payload.BuildFromLoadedRangeTexConfig(labels, fonts, layout);
  return BuildCoreFormatter<TtRangeTexConfigV1, RangeTexConfig,
                            RangeTexFormatter, RangeReportData>(
      payload.GetCConfig(), TT_FORMATTER_CONFIG_KIND_RANGE_TEX, "range latex");
}

auto BuildRangeTypstCoreFormatter(const RangeReportLabels& labels,
                                  const FontConfig& fonts,
                                  const LayoutConfig& layout)
    -> std::unique_ptr<IReportFormatter<RangeReportData>> {
  FormatterConfigPayload payload;
  payload.BuildFromLoadedRangeTypConfig(labels, fonts, layout);
  return BuildCoreFormatter<TtRangeTypConfigV1, RangeTypConfig,
                            RangeTypFormatter, RangeReportData>(
      payload.GetCConfig(), TT_FORMATTER_CONFIG_KIND_RANGE_TYP, "range typst");
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

auto BuildPeriodMarkdownCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<PeriodReportData>> {
  return std::make_unique<RangeReportFormatterAdapter<PeriodReportData>>(
      BuildRangeMarkdownCoreFormatter(
          catalog.loaded_reports.markdown.period.labels));
}

auto BuildWeeklyMarkdownCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<WeeklyReportData>> {
  return std::make_unique<RangeReportFormatterAdapter<WeeklyReportData>>(
      BuildRangeMarkdownCoreFormatter(
          catalog.loaded_reports.markdown.week.labels));
}

auto BuildYearlyMarkdownCoreFormatter(const ReportCatalog& catalog)
    -> std::unique_ptr<IReportFormatter<YearlyReportData>> {
  return std::make_unique<RangeReportFormatterAdapter<YearlyReportData>>(
      BuildRangeMarkdownCoreFormatter(
          catalog.loaded_reports.markdown.year.labels));
}

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
using AndroidPolicy =
    infrastructure::reports::AndroidStaticReportFormatterPolicy;

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

constexpr std::array<FormatRegistrationRow, 3> kFormatRegistrationRows = {{
    {
        .format = ReportFormat::kMarkdown,
        .enabled_flag = &AndroidPolicy::enable_markdown,
        .disabled_reason = "Markdown formatter is disabled by Android static "
                           "formatter policy.",
        .build_daily = &BuildDayMarkdownCoreFormatter,
        .build_monthly = &BuildMonthMarkdownCoreFormatter,
        .build_period = &BuildPeriodMarkdownCoreFormatter,
        .build_weekly = &BuildWeeklyMarkdownCoreFormatter,
        .build_yearly = &BuildYearlyMarkdownCoreFormatter,
    },
    {
        .format = ReportFormat::kLaTeX,
        .enabled_flag = &AndroidPolicy::enable_latex,
        .disabled_reason =
            "LaTeX formatter is disabled by Android static formatter policy.",
        .build_daily = &BuildDayLatexCoreFormatter,
        .build_monthly = &BuildMonthLatexCoreFormatter,
        .build_period = &BuildPeriodLatexCoreFormatter,
        .build_weekly = &BuildWeeklyLatexCoreFormatter,
        .build_yearly = &BuildYearlyLatexCoreFormatter,
    },
    {
        .format = ReportFormat::kTyp,
        .enabled_flag = &AndroidPolicy::enable_typst,
        .disabled_reason =
            "Typst formatter is disabled by Android static formatter policy.",
        .build_daily = &BuildDayTypstCoreFormatter,
        .build_monthly = &BuildMonthTypstCoreFormatter,
        .build_period = &BuildPeriodTypstCoreFormatter,
        .build_weekly = &BuildWeeklyTypstCoreFormatter,
        .build_yearly = &BuildYearlyTypstCoreFormatter,
    },
}};

auto RegisterCreatorsForRow(const FormatRegistrationRow& row) -> void {
  RegisterCoreCreator<DailyReportData>(row.format, row.build_daily);
  RegisterCoreCreator<MonthlyReportData>(row.format, row.build_monthly);
  RegisterCoreCreator<PeriodReportData>(row.format, row.build_period);
  RegisterCoreCreator<WeeklyReportData>(row.format, row.build_weekly);
  RegisterCoreCreator<YearlyReportData>(row.format, row.build_yearly);
}

}  // namespace

namespace infrastructure::reports {

AndroidStaticReportFormatterRegistrar::AndroidStaticReportFormatterRegistrar(
    AndroidStaticReportFormatterPolicy policy)
    : policy_(policy) {}

auto AndroidStaticReportFormatterRegistrar::RegisterStaticFormatters() const
    -> void {
  for (const auto& row : kFormatRegistrationRows) {
    if (policy_.*(row.enabled_flag)) {
      RegisterCreatorsForRow(row);
      continue;
    }
    RegisterDisabledFormatForAllTypes(row.format, row.disabled_reason);
  }
}

}  // namespace infrastructure::reports
