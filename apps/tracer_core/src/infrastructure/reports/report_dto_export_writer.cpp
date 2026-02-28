// infrastructure/reports/report_dto_export_writer.cpp
#include "infrastructure/reports/report_dto_export_writer.hpp"

#include <algorithm>
#include <cctype>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace infrastructure::reports {

namespace {

constexpr int kMinMonth = 1;
constexpr int kMaxMonth = 12;
constexpr int kMinIsoWeek = 1;
constexpr int kMaxIsoWeek = 53;

constexpr size_t kYearWidth = 4;
constexpr size_t kMonthWidth = 2;

constexpr size_t kDashedDateMinLength = 10;
constexpr size_t kCompactDateMinLength = 8;
constexpr size_t kDateDashYearMonthPos = 4;
constexpr size_t kDateDashMonthDayPos = 7;

constexpr size_t kDashedMonthLabelMinLength = 7;
constexpr size_t kCompactMonthLabelMinLength = 6;
constexpr size_t kMonthLabelDashPos = 4;

constexpr size_t kMonthOffsetWithDash = 5;
constexpr size_t kMonthOffsetWithoutDash = 4;

constexpr std::string_view kIsoWeekSeparator = "-W";
constexpr size_t kIsoWeekSeparatorLength = 2;

struct YearMonthParts {
  int year;
  int month;
};

struct YearWeekParts {
  int year;
  int week;
};

auto IsAllDigits(std::string_view value) -> bool {
  if (value.empty()) {
    return false;
  }
  return std::ranges::all_of(value, [](unsigned char kCharacter) -> bool {
    return std::isdigit(kCharacter) != 0;
  });
}

auto ParseYearMonthViews(std::string_view kYearView,
                         std::string_view kMonthView)
    -> std::optional<YearMonthParts> {
  if (!IsAllDigits(kYearView) || !IsAllDigits(kMonthView)) {
    return std::nullopt;
  }

  try {
    YearMonthParts parsed{.year = std::stoi(std::string(kYearView)),
                          .month = std::stoi(std::string(kMonthView))};
    if (parsed.month < kMinMonth || parsed.month > kMaxMonth) {
      return std::nullopt;
    }
    return parsed;
  } catch (...) {
    return std::nullopt;
  }
}

auto ParseDateYearMonth(const std::string& date)
    -> std::optional<YearMonthParts> {
  if (date.size() >= kDashedDateMinLength &&
      date[kDateDashYearMonthPos] == '-' && date[kDateDashMonthDayPos] == '-') {
    const std::string_view kYearView(date.data(), kYearWidth);
    const std::string_view kMonthView(date.data() + kMonthOffsetWithDash,
                                      kMonthWidth);
    return ParseYearMonthViews(kYearView, kMonthView);
  }

  if (date.size() >= kCompactDateMinLength) {
    const std::string_view kYearView(date.data(), kYearWidth);
    const std::string_view kMonthView(date.data() + kMonthOffsetWithoutDash,
                                      kMonthWidth);
    return ParseYearMonthViews(kYearView, kMonthView);
  }

  return std::nullopt;
}

auto ParseYearMonthLabel(const std::string& label)
    -> std::optional<YearMonthParts> {
  if (label.size() >= kDashedMonthLabelMinLength &&
      label[kMonthLabelDashPos] == '-') {
    const std::string_view kYearView(label.data(), kYearWidth);
    const std::string_view kMonthView(label.data() + kMonthOffsetWithDash,
                                      kMonthWidth);
    return ParseYearMonthViews(kYearView, kMonthView);
  }

  if (label.size() >= kCompactMonthLabelMinLength) {
    const std::string_view kYearView(label.data(), kYearWidth);
    const std::string_view kMonthView(label.data() + kMonthOffsetWithoutDash,
                                      kMonthWidth);
    return ParseYearMonthViews(kYearView, kMonthView);
  }

  return std::nullopt;
}

auto ParseIsoWeekLabel(const std::string& label)
    -> std::optional<YearWeekParts> {
  const size_t kSeparatorPos = label.find(kIsoWeekSeparator);
  if (kSeparatorPos == std::string::npos || kSeparatorPos == 0 ||
      kSeparatorPos + kIsoWeekSeparatorLength >= label.size()) {
    return std::nullopt;
  }

  const std::string_view kYearView(label.data(), kSeparatorPos);
  const std::string_view kWeekView(
      label.data() + kSeparatorPos + kIsoWeekSeparatorLength,
      label.size() - kSeparatorPos - kIsoWeekSeparatorLength);
  if (!IsAllDigits(kYearView) || !IsAllDigits(kWeekView)) {
    return std::nullopt;
  }

  try {
    YearWeekParts parsed{.year = std::stoi(std::string(kYearView)),
                         .week = std::stoi(std::string(kWeekView))};
    if (parsed.week < kMinIsoWeek || parsed.week > kMaxIsoWeek) {
      return std::nullopt;
    }
    return parsed;
  } catch (...) {
    return std::nullopt;
  }
}

auto ParseYearLabel(const std::string& label, int& year) -> bool {
  if (!IsAllDigits(label)) {
    return false;
  }
  try {
    year = std::stoi(label);
    return true;
  } catch (...) {
    return false;
  }
}

}  // namespace

ReportDtoExportWriter::ReportDtoExportWriter(
    std::shared_ptr<tracer_core::application::ports::IReportDtoFormatter>
        formatter,
    std::shared_ptr<IReportExporter> exporter)
    : formatter_(std::move(formatter)), exporter_(std::move(exporter)) {
  if (!formatter_) {
    throw std::invalid_argument("formatter must not be null.");
  }
  if (!exporter_) {
    throw std::invalid_argument("exporter must not be null.");
  }
}

auto ReportDtoExportWriter::ExportSingleDay(const std::string& date,
                                            const DailyReportData& report,
                                            ReportFormat format) -> void {
  const std::string kContent = formatter_->FormatDaily(report, format);
  exporter_->ExportSingleDayReport({.id = date, .kContent = kContent}, format);
}

auto ReportDtoExportWriter::ExportSingleMonth(const std::string& month,
                                              const MonthlyReportData& report,
                                              ReportFormat format) -> void {
  const std::string kContent = formatter_->FormatMonthly(report, format);
  exporter_->ExportSingleMonthReport({.id = month, .kContent = kContent},
                                     format);
}

auto ReportDtoExportWriter::ExportSinglePeriod(int days,
                                               const PeriodReportData& report,
                                               ReportFormat format) -> void {
  const std::string kContent = formatter_->FormatPeriod(report, format);
  exporter_->ExportSinglePeriodReport(days, kContent, format);
}

auto ReportDtoExportWriter::ExportSingleWeek(const std::string& iso_week,
                                             const WeeklyReportData& report,
                                             ReportFormat format) -> void {
  const std::string kContent = formatter_->FormatWeekly(report, format);
  exporter_->ExportSingleWeekReport({.id = iso_week, .kContent = kContent},
                                    format);
}

auto ReportDtoExportWriter::ExportSingleYear(const std::string& year,
                                             const YearlyReportData& report,
                                             ReportFormat format) -> void {
  const std::string kContent = formatter_->FormatYearly(report, format);
  exporter_->ExportSingleYearReport({.id = year, .kContent = kContent}, format);
}

auto ReportDtoExportWriter::ExportAllDaily(
    const std::map<std::string, DailyReportData>& reports, ReportFormat format)
    -> void {
  FormattedGroupedReports formatted_reports;
  for (const auto& [date, report] : reports) {
    const auto kParsedDate = ParseDateYearMonth(date);
    if (!kParsedDate.has_value()) {
      continue;
    }
    formatted_reports[kParsedDate->year][kParsedDate->month].push_back(
        {.report_id = date,
         .kContent = formatter_->FormatDaily(report, format)});
  }
  exporter_->ExportAllDailyReports(formatted_reports, format);
}

auto ReportDtoExportWriter::ExportAllMonthly(
    const std::map<std::string, MonthlyReportData>& reports,
    ReportFormat format) -> void {
  FormattedMonthlyReports formatted_reports;
  for (const auto& [label, report] : reports) {
    const auto kParsedLabel = ParseYearMonthLabel(label);
    if (!kParsedLabel.has_value()) {
      continue;
    }
    formatted_reports[kParsedLabel->year][kParsedLabel->month] =
        formatter_->FormatMonthly(report, format);
  }
  exporter_->ExportAllMonthlyReports(formatted_reports, format);
}

auto ReportDtoExportWriter::ExportAllPeriod(
    const std::map<int, PeriodReportData>& reports, ReportFormat format)
    -> void {
  FormattedPeriodReports formatted_reports;
  for (const auto& [days, report] : reports) {
    formatted_reports[days] = formatter_->FormatPeriod(report, format);
  }
  exporter_->ExportAllPeriodReports(formatted_reports, format);
}

auto ReportDtoExportWriter::ExportAllWeekly(
    const std::map<std::string, WeeklyReportData>& reports, ReportFormat format)
    -> void {
  FormattedWeeklyReports formatted_reports;
  for (const auto& [label, report] : reports) {
    const auto kParsedLabel = ParseIsoWeekLabel(label);
    if (!kParsedLabel.has_value()) {
      continue;
    }
    formatted_reports[kParsedLabel->year][kParsedLabel->week] =
        formatter_->FormatWeekly(report, format);
  }
  exporter_->ExportAllWeeklyReports(formatted_reports, format);
}

auto ReportDtoExportWriter::ExportAllYearly(
    const std::map<std::string, YearlyReportData>& reports, ReportFormat format)
    -> void {
  FormattedYearlyReports formatted_reports;
  for (const auto& [label, report] : reports) {
    int year = 0;
    if (!ParseYearLabel(label, year)) {
      continue;
    }
    formatted_reports[year] = formatter_->FormatYearly(report, format);
  }
  exporter_->ExportAllYearlyReports(formatted_reports, format);
}

}  // namespace infrastructure::reports
