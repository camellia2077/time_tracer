// infrastructure/reports/report_file_manager.cpp
#include "infrastructure/reports/report_file_manager.hpp"

#include <algorithm>
#include <cctype>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "infrastructure/reports/export_utils.hpp"

namespace {

struct DailyPathLayout {
  std::string year;
  std::string month;
  std::string file_stem;
};

auto IsAllDigits(std::string_view value) -> bool {
  return !value.empty() &&
         std::ranges::all_of(value, [](const char character) -> bool {
           return std::isdigit(static_cast<unsigned char>(character)) != 0;
         });
}

auto BuildDailyPathLayoutFromCompact(std::string_view value)
    -> std::optional<DailyPathLayout> {
  constexpr size_t kCompactDateLength = 8;
  constexpr size_t kYearOffset = 0;
  constexpr size_t kMonthOffset = 4;
  constexpr size_t kDayOffset = 6;
  constexpr size_t kYearLength = 4;
  constexpr size_t kMonthLength = 2;
  constexpr size_t kDayLength = 2;
  if (value.size() != kCompactDateLength || !IsAllDigits(value)) {
    return std::nullopt;
  }

  const std::string kYear = std::string(value.substr(kYearOffset, kYearLength));
  const std::string kMonth =
      std::string(value.substr(kMonthOffset, kMonthLength));
  const std::string kDay = std::string(value.substr(kDayOffset, kDayLength));
  return DailyPathLayout{
      .year = kYear,
      .month = kMonth,
      .file_stem = kYear + "-" + kMonth + "-" + kDay,
  };
}

auto BuildDailyPathLayoutFromIso(std::string_view value)
    -> std::optional<DailyPathLayout> {
  constexpr size_t kIsoDateLength = 10;
  constexpr size_t kYearOffset = 0;
  constexpr size_t kMonthOffset = 5;
  constexpr size_t kDayOffset = 8;
  constexpr size_t kYearLength = 4;
  constexpr size_t kMonthLength = 2;
  constexpr size_t kDayLength = 2;
  constexpr size_t kFirstHyphenOffset = 4;
  constexpr size_t kSecondHyphenOffset = 7;
  if (value.size() != kIsoDateLength || value[kFirstHyphenOffset] != '-' ||
      value[kSecondHyphenOffset] != '-') {
    return std::nullopt;
  }

  const std::string_view kYear = value.substr(kYearOffset, kYearLength);
  const std::string_view kMonth = value.substr(kMonthOffset, kMonthLength);
  const std::string_view kDay = value.substr(kDayOffset, kDayLength);
  if (!IsAllDigits(kYear) || !IsAllDigits(kMonth) || !IsAllDigits(kDay)) {
    return std::nullopt;
  }

  return DailyPathLayout{
      .year = std::string(kYear),
      .month = std::string(kMonth),
      .file_stem = std::string(value),
  };
}

auto ResolveDailyPathLayout(std::string_view date)
    -> std::optional<DailyPathLayout> {
  if (auto iso = BuildDailyPathLayoutFromIso(date); iso.has_value()) {
    return iso;
  }
  return BuildDailyPathLayoutFromCompact(date);
}

}  // namespace

ReportFileManager::ReportFileManager(fs::path export_root)
    : export_root_path_(std::move(export_root)) {}

auto ReportFileManager::GetSingleDayReportPath(const std::string& date,
                                               ReportFormat format) const
    -> fs::path {
  auto details_opt = ExportUtils::GetReportFormatDetails(format);
  if (!details_opt.has_value()) {
    throw std::runtime_error("Unsupported report format.");
  }
  auto details = details_opt.value();
  fs::path day_root = export_root_path_ / details.dir_name / "day";
  if (auto layout = ResolveDailyPathLayout(date); layout.has_value()) {
    return day_root / layout->year / layout->month /
           (layout->file_stem + details.extension);
  }
  return day_root / (date + details.extension);
}

auto ReportFileManager::GetSingleMonthReportPath(const std::string& month,
                                                 ReportFormat format) const
    -> fs::path {
  auto details_opt = ExportUtils::GetReportFormatDetails(format);
  if (!details_opt.has_value()) {
    throw std::runtime_error("Unsupported report format.");
  }
  auto details = details_opt.value();
  return export_root_path_ / details.dir_name / "month" /
         (month + details.extension);
}

auto ReportFileManager::GetSinglePeriodReportPath(int days,
                                                  ReportFormat format) const
    -> fs::path {
  auto details_opt = ExportUtils::GetReportFormatDetails(format);
  if (!details_opt.has_value()) {
    throw std::runtime_error("Unsupported report format.");
  }
  auto details = details_opt.value();
  return export_root_path_ / details.dir_name / "recent" /
         ("last_" + std::to_string(days) + "_days_report" + details.extension);
}

auto ReportFileManager::GetSingleWeekReportPath(const std::string& iso_week,
                                                ReportFormat format) const
    -> fs::path {
  auto details_opt = ExportUtils::GetReportFormatDetails(format);
  if (!details_opt.has_value()) {
    throw std::runtime_error("Unsupported report format.");
  }
  auto details = details_opt.value();
  return export_root_path_ / details.dir_name / "week" /
         (iso_week + details.extension);
}

auto ReportFileManager::GetSingleYearReportPath(const std::string& year,
                                                ReportFormat format) const
    -> fs::path {
  auto details_opt = ExportUtils::GetReportFormatDetails(format);
  if (!details_opt.has_value()) {
    throw std::runtime_error("Unsupported report format.");
  }
  auto details = details_opt.value();
  return export_root_path_ / details.dir_name / "year" /
         (year + details.extension);
}

auto ReportFileManager::GetAllDailyReportsBaseDir(ReportFormat format) const
    -> fs::path {
  auto details_opt = ExportUtils::GetReportFormatDetails(format);
  if (!details_opt.has_value()) {
    throw std::runtime_error("Unsupported report format.");
  }
  auto details = details_opt.value();
  return export_root_path_ / details.dir_name / "day";
}

auto ReportFileManager::GetAllMonthlyReportsBaseDir(ReportFormat format) const
    -> fs::path {
  auto details_opt = ExportUtils::GetReportFormatDetails(format);
  if (!details_opt.has_value()) {
    throw std::runtime_error("Unsupported report format.");
  }
  auto details = details_opt.value();
  return export_root_path_ / details.dir_name / "month";
}

auto ReportFileManager::GetAllPeriodReportsBaseDir(ReportFormat format) const
    -> fs::path {
  auto details_opt = ExportUtils::GetReportFormatDetails(format);
  if (!details_opt.has_value()) {
    throw std::runtime_error("Unsupported report format.");
  }
  auto details = details_opt.value();
  return export_root_path_ / details.dir_name / "recent";
}

auto ReportFileManager::GetAllWeeklyReportsBaseDir(ReportFormat format) const
    -> fs::path {
  auto details_opt = ExportUtils::GetReportFormatDetails(format);
  if (!details_opt.has_value()) {
    throw std::runtime_error("Unsupported report format.");
  }
  auto details = details_opt.value();
  return export_root_path_ / details.dir_name / "week";
}

auto ReportFileManager::GetAllYearlyReportsBaseDir(ReportFormat format) const
    -> fs::path {
  auto details_opt = ExportUtils::GetReportFormatDetails(format);
  if (!details_opt.has_value()) {
    throw std::runtime_error("Unsupported report format.");
  }
  auto details = details_opt.value();
  return export_root_path_ / details.dir_name / "year";
}
