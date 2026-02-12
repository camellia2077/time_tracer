// infrastructure/reports/monthly/formatters/typst/month_typ_formatter.cpp
#include "infrastructure/reports/monthly/formatters/typst/month_typ_formatter.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <memory>
#include <string>

#include "infrastructure/reports/shared/formatters/typst/typ_utils.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_abi_v2.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_config_bridge_v1.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_report_data_bridge.hpp"
#include "infrastructure/reports/shared/utils/format/report_string_utils.hpp"
#include "infrastructure/reports/shared/utils/format/time_format.hpp"

namespace {

thread_local std::string last_error_message;
thread_local int32_t last_error_code = TT_FORMATTER_STATUS_OK;

constexpr uint16_t kImplementationVersionMajor = 0U;
constexpr uint16_t kImplementationVersionMinor = 5U;
constexpr uint16_t kImplementationVersionPatch = 8U;
constexpr std::size_t kBulletLineReservePadding = 8;
constexpr std::array<uint32_t, 1> kSupportedReportDataKinds = {
    TT_REPORT_DATA_KIND_MONTHLY};

auto BuildBulletLine(const std::string& label, const std::string& value)
    -> std::string {
  std::string line;
  line.reserve(label.size() + value.size() + kBulletLineReservePadding);
  line += "+ *";
  line += label;
  line += ":* ";
  line += value;
  return line;
}

auto FormatRatio(int count, int total_days) -> std::string {
  return FormatCountWithPercentage(count, total_days);
}

auto BuildMonthlySummaryData(const TtRangeReportDataV1& data_view)
    -> MonthlyReportData {
  MonthlyReportData data{};
  if ((data_view.rangeLabel.data != nullptr) &&
      (data_view.rangeLabel.length > 0U)) {
    data.range_label.assign(data_view.rangeLabel.data,
                            static_cast<size_t>(data_view.rangeLabel.length));
  }
  if ((data_view.startDate.data != nullptr) &&
      (data_view.startDate.length > 0U)) {
    data.start_date.assign(data_view.startDate.data,
                           static_cast<size_t>(data_view.startDate.length));
  }
  if ((data_view.endDate.data != nullptr) && (data_view.endDate.length > 0U)) {
    data.end_date.assign(data_view.endDate.data,
                         static_cast<size_t>(data_view.endDate.length));
  }
  data.requested_days = data_view.requestedDays;
  data.total_duration = data_view.totalDuration;
  data.actual_days = data_view.actualDays;
  data.status_true_days = data_view.statusTrueDays;
  data.sleep_true_days = data_view.sleepTrueDays;
  data.exercise_true_days = data_view.exerciseTrueDays;
  data.cardio_true_days = data_view.cardioTrueDays;
  data.anaerobic_true_days = data_view.anaerobicTrueDays;
  data.is_valid = (data_view.isValid != 0U);
  return data;
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
                         MonthTypFormatter** formatter_out)
    -> TtFormatterStatusCode {
  if ((formatter_config == nullptr) || (formatter_out == nullptr)) {
    SetLastError(TT_FORMATTER_STATUS_INVALID_ARGUMENT,
                 "Invalid arguments for formatter creation.");
    return TT_FORMATTER_STATUS_INVALID_ARGUMENT;
  }

  try {
    const TtMonthTypConfigV1* config_view = nullptr;
    std::string config_error;
    if (!formatter_c_config_bridge_v1::GetMonthTypConfigView(
            formatter_config, &config_view, &config_error)) {
      SetLastError(TT_FORMATTER_STATUS_CONFIG_ERROR, config_error);
      *formatter_out = nullptr;
      return TT_FORMATTER_STATUS_CONFIG_ERROR;
    }
    auto typ_config = std::make_shared<MonthTypConfig>(*config_view);
    auto formatter = std::make_unique<MonthTypFormatter>(typ_config);
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

auto FormatReportImpl(MonthTypFormatter* formatter,
                      const TtRangeReportDataV1* data_view,
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

MonthTypFormatter::MonthTypFormatter(std::shared_ptr<MonthTypConfig> config)
    : BaseTypFormatter(config) {}

auto MonthTypFormatter::FormatReportFromView(
    const TtRangeReportDataV1& data_view) const -> std::string {
  MonthlyReportData summary_data = BuildMonthlySummaryData(data_view);

  std::string report_stream;
  FormatPageSetup(report_stream);
  FormatTextSetup(report_stream);

  if (std::string error_message = ValidateData(summary_data);
      !error_message.empty()) {
    report_stream += error_message;
    report_stream += "\n";
    return report_stream;
  }

  FormatHeaderContent(report_stream, summary_data);

  if (IsEmptyData(summary_data)) {
    report_stream += GetNoRecordsMsg();
    report_stream += "\n";
    return report_stream;
  }

  report_stream += TypUtils::BuildTitleText(
      config_->GetCategoryTitleFont(), config_->GetCategoryTitleFontSize(),
      config_->GetProjectBreakdownLabel());
  report_stream += "\n\n";
  report_stream += TypUtils::FormatProjectTree(
      data_view.projectTreeNodes, data_view.projectTreeNodeCount,
      data_view.totalDuration, GetAvgDays(summary_data),
      config_->GetCategoryTitleFont(), config_->GetCategoryTitleFontSize());
  return report_stream;
}

auto MonthTypFormatter::ValidateData(const MonthlyReportData& data) const
    -> std::string {
  if (!data.is_valid) {
    return config_->GetInvalidFormatMessage();
  }
  return "";
}

auto MonthTypFormatter::IsEmptyData(const MonthlyReportData& data) const
    -> bool {
  return data.actual_days == 0;
}

auto MonthTypFormatter::GetAvgDays(const MonthlyReportData& data) const -> int {
  return data.actual_days;
}

auto MonthTypFormatter::GetNoRecordsMsg() const -> std::string {
  return config_->GetNoRecordsMessage();
}

void MonthTypFormatter::FormatPageSetup(std::string& report_stream) const {
  report_stream += TypUtils::BuildPageSetup(
      config_->GetMarginTopCm(), config_->GetMarginBottomCm(),
      config_->GetMarginLeftCm(), config_->GetMarginRightCm());
  report_stream += "\n";
}

void MonthTypFormatter::FormatHeaderContent(
    std::string& report_stream, const MonthlyReportData& data) const {
  std::string title_text =
      FormatTitleTemplate(config_->GetTitleTemplate(), data);
  report_stream += TypUtils::BuildTitleText(
      config_->GetTitleFont(), config_->GetReportTitleFontSize(), title_text);
  report_stream += "\n\n";

  if (data.actual_days > 0) {
    report_stream += BuildBulletLine(config_->GetActualDaysLabel(),
                                     std::to_string(data.actual_days));
    report_stream += "\n";
    report_stream += BuildBulletLine(
        config_->GetTotalTimeLabel(),
        TimeFormatDuration(data.total_duration, data.actual_days));
    report_stream += "\n";
    report_stream +=
        BuildBulletLine(config_->GetStatusDaysLabel(),
                        FormatRatio(data.status_true_days, data.actual_days));
    report_stream += "\n";
    report_stream +=
        BuildBulletLine(config_->GetSleepDaysLabel(),
                        FormatRatio(data.sleep_true_days, data.actual_days));
    report_stream += "\n";
    report_stream +=
        BuildBulletLine(config_->GetExerciseDaysLabel(),
                        FormatRatio(data.exercise_true_days, data.actual_days));
    report_stream += "\n";
    report_stream +=
        BuildBulletLine(config_->GetCardioDaysLabel(),
                        FormatRatio(data.cardio_true_days, data.actual_days));
    report_stream += "\n";
    report_stream += BuildBulletLine(
        config_->GetAnaerobicDaysLabel(),
        FormatRatio(data.anaerobic_true_days, data.actual_days));
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

  MonthTypFormatter* formatter = nullptr;
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
    std::unique_ptr<MonthTypFormatter>{static_cast<MonthTypFormatter*>(handle)};
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
          kSupportedReportDataKinds.size(), "month typst", nullptr,
          &native_report_data, &validation_error_code,
          &validation_error_message)) {
    SetLastError(validation_error_code, validation_error_message);
    return validation_error_code;
  }

  auto* formatter = static_cast<MonthTypFormatter*>(handle);
  const TtRangeReportDataV1* month_data_view = nullptr;
  if (!formatter_c_report_data_bridge::ValidateRangeReportDataView(
          native_report_data, &month_data_view, &validation_error_message)) {
    SetLastError(TT_FORMATTER_STATUS_INVALID_ARGUMENT,
                 validation_error_message);
    return TT_FORMATTER_STATUS_INVALID_ARGUMENT;
  }

  std::string formatted_report;
  auto status_code =
      FormatReportImpl(formatter, month_data_view, &formatted_report);
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
