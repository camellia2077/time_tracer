// infrastructure/reports/daily/formatters/markdown/day_md_formatter.cpp
#include "infrastructure/reports/daily/formatters/markdown/day_md_formatter.hpp"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>

#include "infrastructure/reports/daily/formatters/common/day_report_view_utils.hpp"
#include "infrastructure/reports/daily/formatters/statistics/markdown_stat_strategy.hpp"
#include "infrastructure/reports/daily/formatters/statistics/stat_formatter.hpp"
#include "infrastructure/reports/shared/formatters/markdown/markdown_formatter.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_abi_v2.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_config_bridge_v1.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_report_data_bridge.hpp"
#include "infrastructure/reports/shared/utils/format/bool_to_string.hpp"
#include "infrastructure/reports/shared/utils/format/report_string_utils.hpp"
#include "infrastructure/reports/shared/utils/format/time_format.hpp"

namespace {
constexpr int kRemarkIndent = 2;
const std::string kRemarkIndentPrefix = "  ";
constexpr size_t kMarkdownItemLinePadding = 8;
constexpr size_t kActivityLinePadding = 32;

thread_local std::string last_error_message;
thread_local int32_t last_error_code = TT_FORMATTER_STATUS_OK;

constexpr uint16_t kImplementationVersionMajor = 0U;
constexpr uint16_t kImplementationVersionMinor = 5U;
constexpr uint16_t kImplementationVersionPatch = 8U;
constexpr std::array<uint32_t, 1> kSupportedReportDataKinds = {
    TT_REPORT_DATA_KIND_DAILY};

auto BuildMarkdownItemLine(const std::string& label, const std::string& value)
    -> std::string {
  std::string line;
  line.reserve(label.size() + value.size() + kMarkdownItemLinePadding);
  line += "- **";
  line += label;
  line += "**: ";
  line += value;
  line += "\n";
  return line;
}

auto BuildActivityLine(const TimeRecord& record,
                       const std::string& project_path) -> std::string {
  std::string line;
  line.reserve(record.start_time.size() + record.end_time.size() +
               project_path.size() + kActivityLinePadding);
  line += "- ";
  line += record.start_time;
  line += " - ";
  line += record.end_time;
  line += " (";
  line += TimeFormatDuration(record.duration_seconds);
  line += "): ";
  line += project_path;
  line += "\n";
  return line;
}

void SetLastError(int32_t error_code, const std::string& error_message) {
  last_error_code = error_code;
  last_error_message = error_message;
}

void ClearLastError() {
  last_error_code = TT_FORMATTER_STATUS_OK;
  last_error_message.clear();
}

auto CreateFormatterImpl(const TtFormatterConfig* formatter_config,
                         DayMdFormatter** formatter_out)
    -> TtFormatterStatusCode {
  if ((formatter_config == nullptr) || (formatter_out == nullptr)) {
    SetLastError(TT_FORMATTER_STATUS_INVALID_ARGUMENT,
                 "Invalid arguments for formatter creation.");
    return TT_FORMATTER_STATUS_INVALID_ARGUMENT;
  }

  try {
    const TtDayMdConfigV1* config_view = nullptr;
    std::string config_error;
    if (!formatter_c_config_bridge_v1::GetDayMdConfigView(
            formatter_config, &config_view, &config_error)) {
      SetLastError(TT_FORMATTER_STATUS_CONFIG_ERROR, config_error);
      *formatter_out = nullptr;
      return TT_FORMATTER_STATUS_CONFIG_ERROR;
    }
    auto md_config = std::make_shared<DayMdConfig>(*config_view);
    auto formatter = std::make_unique<DayMdFormatter>(md_config);
    *formatter_out = formatter.release();
    ClearLastError();
    return TT_FORMATTER_STATUS_OK;
  } catch (const std::exception& exception) {
    SetLastError(TT_FORMATTER_STATUS_INTERNAL_ERROR, exception.what());
    *formatter_out = nullptr;
    return TT_FORMATTER_STATUS_INTERNAL_ERROR;
  } catch (...) {
    SetLastError(TT_FORMATTER_STATUS_INTERNAL_ERROR,
                 "Unknown error while creating formatter.");
    *formatter_out = nullptr;
    return TT_FORMATTER_STATUS_INTERNAL_ERROR;
  }
}

auto FormatReportImpl(DayMdFormatter* formatter,
                      const TtDailyReportDataV1* data_view,
                      std::string* formatted_report) -> TtFormatterStatusCode {
  if ((formatter == nullptr) || (data_view == nullptr) ||
      (formatted_report == nullptr)) {
    SetLastError(TT_FORMATTER_STATUS_INVALID_ARGUMENT,
                 "Invalid arguments for report formatting.");
    return TT_FORMATTER_STATUS_INVALID_ARGUMENT;
  }

  try {
    *formatted_report = formatter->FormatReportFromView(*data_view);
    ClearLastError();
    return TT_FORMATTER_STATUS_OK;
  } catch (const std::exception& exception) {
    SetLastError(TT_FORMATTER_STATUS_FORMAT_ERROR, exception.what());
    return TT_FORMATTER_STATUS_FORMAT_ERROR;
  } catch (...) {
    SetLastError(TT_FORMATTER_STATUS_FORMAT_ERROR,
                 "Unknown error while formatting report.");
    return TT_FORMATTER_STATUS_FORMAT_ERROR;
  }
}
}  // namespace

DayMdFormatter::DayMdFormatter(std::shared_ptr<DayMdConfig> config)
    : BaseMdFormatter(config) {}

auto DayMdFormatter::FormatReportFromView(
    const TtDailyReportDataV1& data_view) const -> std::string {
  DailyReportData data =
      day_report_view_utils::BuildDailyContentData(data_view);

  std::string report_stream;
  FormatHeaderContent(report_stream, data);

  if (IsEmptyData(data)) {
    report_stream += GetNoRecordsMsg();
    report_stream += "\n";
    return report_stream;
  }

  FormatExtraContent(report_stream, data);
  report_stream += "\n## ";
  report_stream += config_->GetProjectBreakdownLabel();
  report_stream += "\n";
  report_stream += MarkdownFormatter::FormatProjectTree(
      data_view.projectTreeNodes, data_view.projectTreeNodeCount,
      data_view.totalDuration, GetAvgDays(data));
  return report_stream;
}

auto DayMdFormatter::IsEmptyData(const DailyReportData& data) const -> bool {
  return data.total_duration == 0;
}

auto DayMdFormatter::GetAvgDays(const DailyReportData& /*data*/) const -> int {
  return 1;
}

auto DayMdFormatter::GetNoRecordsMsg() const -> std::string {
  return config_->GetNoRecords();
}

void DayMdFormatter::FormatHeaderContent(std::string& report_stream,
                                         const DailyReportData& data) const {
  report_stream += "## ";
  report_stream += config_->GetTitlePrefix();
  report_stream += " ";
  report_stream += data.date;
  report_stream += "\n\n";
  report_stream += BuildMarkdownItemLine(config_->GetDateLabel(), data.date);
  report_stream += BuildMarkdownItemLine(
      config_->GetTotalTimeLabel(), TimeFormatDuration(data.total_duration));
  report_stream += BuildMarkdownItemLine(config_->GetStatusLabel(),
                                         BoolToString(data.metadata.status));
  report_stream += BuildMarkdownItemLine(config_->GetSleepLabel(),
                                         BoolToString(data.metadata.sleep));
  report_stream += BuildMarkdownItemLine(config_->GetExerciseLabel(),
                                         BoolToString(data.metadata.exercise));
  report_stream += BuildMarkdownItemLine(config_->GetGetupTimeLabel(),
                                         data.metadata.getup_time);

  std::string formatted_remark = FormatMultilineForList(
      data.metadata.remark, kRemarkIndent, kRemarkIndentPrefix);
  report_stream +=
      BuildMarkdownItemLine(config_->GetRemarkLabel(), formatted_remark);
}

void DayMdFormatter::FormatExtraContent(std::string& report_stream,
                                        const DailyReportData& data) const {
  auto strategy = std::make_unique<MarkdownStatStrategy>();
  StatFormatter stats_formatter(std::move(strategy));
  report_stream += stats_formatter.Format(data, config_);
  DisplayDetailedActivities(report_stream, data);
}

void DayMdFormatter::DisplayDetailedActivities(
    std::string& report_stream, const DailyReportData& data) const {
  if (!data.detailed_records.empty()) {
    report_stream += "\n## ";
    report_stream += config_->GetAllActivitiesLabel();
    report_stream += "\n\n";
    for (const auto& record : data.detailed_records) {
      std::string project_path =
          ReplaceAll(record.project_path, "_", config_->GetActivityConnector());
      report_stream += BuildActivityLine(record, project_path);
      if (record.activityRemark.has_value()) {
        report_stream += "  - **";
        report_stream += config_->GetActivityRemarkLabel();
        report_stream += "**: ";
        report_stream += record.activityRemark.value();
        report_stream += "\n";
      }
    }
    report_stream += "\n";
  }
}

// NOLINTBEGIN(readability-identifier-naming)
extern "C" {
__declspec(dllexport) auto tt_getFormatterAbiInfo(TtFormatterAbiInfo* out_abi)
    -> int32_t {
  if (out_abi == nullptr) {
    return TT_FORMATTER_STATUS_INVALID_ARGUMENT;
  }
  out_abi->structSize = static_cast<uint32_t>(sizeof(TtFormatterAbiInfo));
  out_abi->abiVersion = TT_FORMATTER_ABI_VERSION_CURRENT;
  out_abi->implementationVersion.major = kImplementationVersionMajor;
  out_abi->implementationVersion.minor = kImplementationVersionMinor;
  out_abi->implementationVersion.patch = kImplementationVersionPatch;
  out_abi->implementationVersion.reserved = 0U;
  return TT_FORMATTER_STATUS_OK;
}

__declspec(dllexport) auto tt_createFormatter(const TtFormatterConfig* config,
                                              TtFormatterHandle* out_handle)
    -> int32_t {
  if (out_handle == nullptr) {
    SetLastError(TT_FORMATTER_STATUS_INVALID_ARGUMENT,
                 "out_handle must not be null.");
    return TT_FORMATTER_STATUS_INVALID_ARGUMENT;
  }

  DayMdFormatter* formatter = nullptr;
  auto status_code = CreateFormatterImpl(config, &formatter);
  if (status_code != TT_FORMATTER_STATUS_OK) {
    *out_handle = nullptr;
    return status_code;
  }

  *out_handle = static_cast<TtFormatterHandle>(formatter);
  return TT_FORMATTER_STATUS_OK;
}

__declspec(dllexport) auto tt_destroyFormatter(TtFormatterHandle handle)
    -> int32_t {
  if (handle != nullptr) {
    std::unique_ptr<DayMdFormatter>{static_cast<DayMdFormatter*>(handle)};
  }
  ClearLastError();
  return TT_FORMATTER_STATUS_OK;
}

__declspec(dllexport) auto tt_formatReport(TtFormatterHandle handle,
                                           const void* report_data,
                                           uint32_t report_data_kind,
                                           char** out_report_content,
                                           uint64_t* out_report_size)
    -> int32_t {
  if ((out_report_content == nullptr) || (out_report_size == nullptr)) {
    SetLastError(TT_FORMATTER_STATUS_INVALID_ARGUMENT,
                 "Output pointers must not be null.");
    return TT_FORMATTER_STATUS_INVALID_ARGUMENT;
  }
  *out_report_content = nullptr;
  *out_report_size = 0;

  if ((handle == nullptr) || (report_data == nullptr)) {
    SetLastError(TT_FORMATTER_STATUS_INVALID_ARGUMENT,
                 "handle and report_data must not be null.");
    return TT_FORMATTER_STATUS_INVALID_ARGUMENT;
  }

  const void* native_report_data = nullptr;
  int32_t validation_error_code = TT_FORMATTER_STATUS_OK;
  std::string validation_error_message;
  if (!formatter_c_config_bridge_v1::ValidateReportDataView(
          report_data, report_data_kind, kSupportedReportDataKinds.data(),
          kSupportedReportDataKinds.size(), "day markdown", nullptr,
          &native_report_data, &validation_error_code,
          &validation_error_message)) {
    SetLastError(validation_error_code, validation_error_message);
    return validation_error_code;
  }

  auto* formatter = static_cast<DayMdFormatter*>(handle);
  const TtDailyReportDataV1* day_data_view = nullptr;
  if (!formatter_c_report_data_bridge::ValidateDailyReportDataView(
          native_report_data, &day_data_view, &validation_error_message)) {
    SetLastError(TT_FORMATTER_STATUS_INVALID_ARGUMENT,
                 validation_error_message);
    return TT_FORMATTER_STATUS_INVALID_ARGUMENT;
  }

  std::string formatted_report;
  auto status_code =
      FormatReportImpl(formatter, day_data_view, &formatted_report);
  if (status_code != TT_FORMATTER_STATUS_OK) {
    return status_code;
  }

  auto report_size = formatted_report.size();
  auto* copied_report =
      static_cast<char*>(std::malloc(report_size + static_cast<size_t>(1)));
  if (copied_report == nullptr) {
    SetLastError(TT_FORMATTER_STATUS_MEMORY_ERROR,
                 "Failed to allocate output report buffer.");
    return TT_FORMATTER_STATUS_MEMORY_ERROR;
  }
  if (report_size > 0U) {
    std::memcpy(copied_report, formatted_report.data(), report_size);
  }
  copied_report[report_size] = '\0';

  *out_report_content = copied_report;
  *out_report_size = static_cast<uint64_t>(report_size);
  ClearLastError();
  return TT_FORMATTER_STATUS_OK;
}

__declspec(dllexport) void tt_freeCString(char* c_string) {
  if (c_string != nullptr) {
    std::free(c_string);
  }
}

__declspec(dllexport) auto tt_getLastError(TtFormatterHandle /*handle*/,
                                           TtFormatterError* out_error)
    -> int32_t {
  if (out_error == nullptr) {
    return TT_FORMATTER_STATUS_INVALID_ARGUMENT;
  }
  out_error->structSize = static_cast<uint32_t>(sizeof(TtFormatterError));
  out_error->code = last_error_code;
  out_error->message =
      last_error_message.empty() ? nullptr : last_error_message.c_str();
  return TT_FORMATTER_STATUS_OK;
}
}
// NOLINTEND(readability-identifier-naming)
