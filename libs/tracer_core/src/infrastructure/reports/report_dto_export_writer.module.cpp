module;

#include <algorithm>
#include <cctype>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "application/interfaces/i_report_exporter.hpp"
#include "infrastructure/config/models/report_catalog.hpp"
#include "application/ports/i_report_dto_formatter.hpp"
#include "application/ports/i_report_export_writer.hpp"

module tracer.core.infrastructure.reports.dto.export_writer;

import tracer.core.infrastructure.reports.dto.formatter;

namespace tracer::core::infrastructure::reports {

auto CreateReportDtoFormatter(const ReportCatalog& report_catalog)
    -> std::shared_ptr<tracer_core::application::ports::IReportDtoFormatter> {
  return std::make_shared<ReportDtoFormatter>(report_catalog);
}

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
  return std::ranges::all_of(value, [](unsigned char character) -> bool {
    return std::isdigit(character) != 0;
  });
}

auto ParseYearMonthViews(std::string_view year_view,
                         std::string_view month_view)
    -> std::optional<YearMonthParts> {
  if (!IsAllDigits(year_view) || !IsAllDigits(month_view)) {
    return std::nullopt;
  }

  try {
    YearMonthParts parsed{.year = std::stoi(std::string(year_view)),
                          .month = std::stoi(std::string(month_view))};
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
    const std::string_view year_view(date.data(), kYearWidth);
    const std::string_view month_view(date.data() + kMonthOffsetWithDash,
                                      kMonthWidth);
    return ParseYearMonthViews(year_view, month_view);
  }

  if (date.size() >= kCompactDateMinLength) {
    const std::string_view year_view(date.data(), kYearWidth);
    const std::string_view month_view(date.data() + kMonthOffsetWithoutDash,
                                      kMonthWidth);
    return ParseYearMonthViews(year_view, month_view);
  }

  return std::nullopt;
}

auto ParseYearMonthLabel(const std::string& label)
    -> std::optional<YearMonthParts> {
  if (label.size() >= kDashedMonthLabelMinLength &&
      label[kMonthLabelDashPos] == '-') {
    const std::string_view year_view(label.data(), kYearWidth);
    const std::string_view month_view(label.data() + kMonthOffsetWithDash,
                                      kMonthWidth);
    return ParseYearMonthViews(year_view, month_view);
  }

  if (label.size() >= kCompactMonthLabelMinLength) {
    const std::string_view year_view(label.data(), kYearWidth);
    const std::string_view month_view(label.data() + kMonthOffsetWithoutDash,
                                      kMonthWidth);
    return ParseYearMonthViews(year_view, month_view);
  }

  return std::nullopt;
}

auto ParseIsoWeekLabel(const std::string& label)
    -> std::optional<YearWeekParts> {
  const size_t separator_pos = label.find(kIsoWeekSeparator);
  if (separator_pos == std::string::npos || separator_pos == 0 ||
      separator_pos + kIsoWeekSeparatorLength >= label.size()) {
    return std::nullopt;
  }

  const std::string_view year_view(label.data(), separator_pos);
  const std::string_view week_view(
      label.data() + separator_pos + kIsoWeekSeparatorLength,
      label.size() - separator_pos - kIsoWeekSeparatorLength);
  if (!IsAllDigits(year_view) || !IsAllDigits(week_view)) {
    return std::nullopt;
  }

  try {
    YearWeekParts parsed{.year = std::stoi(std::string(year_view)),
                         .week = std::stoi(std::string(week_view))};
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
  const std::string content = formatter_->FormatDaily(report, format);
  exporter_->ExportSingleDayReport({.id = date, .kContent = content}, format);
}

auto ReportDtoExportWriter::ExportSingleMonth(const std::string& month,
                                              const MonthlyReportData& report,
                                              ReportFormat format) -> void {
  const std::string content = formatter_->FormatMonthly(report, format);
  exporter_->ExportSingleMonthReport({.id = month, .kContent = content},
                                     format);
}

auto ReportDtoExportWriter::ExportSinglePeriod(int days,
                                               const PeriodReportData& report,
                                               ReportFormat format) -> void {
  const std::string content = formatter_->FormatPeriod(report, format);
  exporter_->ExportSinglePeriodReport(days, content, format);
}

auto ReportDtoExportWriter::ExportSingleWeek(const std::string& iso_week,
                                             const WeeklyReportData& report,
                                             ReportFormat format) -> void {
  const std::string content = formatter_->FormatWeekly(report, format);
  exporter_->ExportSingleWeekReport({.id = iso_week, .kContent = content},
                                    format);
}

auto ReportDtoExportWriter::ExportSingleYear(const std::string& year,
                                             const YearlyReportData& report,
                                             ReportFormat format) -> void {
  const std::string content = formatter_->FormatYearly(report, format);
  exporter_->ExportSingleYearReport({.id = year, .kContent = content}, format);
}

auto ReportDtoExportWriter::ExportAllDaily(
    const std::map<std::string, DailyReportData>& reports, ReportFormat format)
    -> void {
  FormattedGroupedReports formatted_reports;
  for (const auto& [date, report] : reports) {
    const auto parsed_date = ParseDateYearMonth(date);
    if (!parsed_date.has_value()) {
      continue;
    }
    formatted_reports[parsed_date->year][parsed_date->month].push_back(
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
    const auto parsed_label = ParseYearMonthLabel(label);
    if (!parsed_label.has_value()) {
      continue;
    }
    formatted_reports[parsed_label->year][parsed_label->month] =
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
    const auto parsed_label = ParseIsoWeekLabel(label);
    if (!parsed_label.has_value()) {
      continue;
    }
    formatted_reports[parsed_label->year][parsed_label->week] =
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

}  // namespace tracer::core::infrastructure::reports
