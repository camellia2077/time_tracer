// infrastructure/reports/daily/formatters/latex/day_tex_formatter.cpp
#include "infrastructure/reports/daily/formatters/latex/day_tex_formatter.hpp"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <memory>
#include <string>

#include "infrastructure/reports/daily/formatters/common/day_report_view_utils.hpp"
#include "infrastructure/reports/daily/formatters/latex/day_tex_utils.hpp"
#include "infrastructure/reports/daily/formatters/statistics/latex_strategy.hpp"
#include "infrastructure/reports/daily/formatters/statistics/stat_formatter.hpp"
#include "infrastructure/reports/shared/formatters/latex/tex_utils.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_abi_v2.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_config_bridge_v1.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_report_data_bridge.hpp"

namespace {

thread_local std::string last_error_message;
thread_local int32_t last_error_code = TT_FORMATTER_STATUS_OK;

constexpr uint16_t kImplementationVersionMajor = 0U;
constexpr uint16_t kImplementationVersionMinor = 5U;
constexpr uint16_t kImplementationVersionPatch = 8U;
constexpr std::array<uint32_t, 1> kSupportedReportDataKinds = {
    TT_REPORT_DATA_KIND_DAILY};

void SetLastError(int32_t error_code, const std::string& error_message) {
  last_error_code = error_code;
  last_error_message = error_message;
}

void ClearLastError() {
  last_error_code = TT_FORMATTER_STATUS_OK;
  last_error_message.clear();
}

auto CreateFormatterImpl(const TtFormatterConfig* formatter_config,
                         DayTexFormatter** formatter_out)
    -> TtFormatterStatusCode {
  if ((formatter_config == nullptr) || (formatter_out == nullptr)) {
    SetLastError(TT_FORMATTER_STATUS_INVALID_ARGUMENT,
                 "Invalid arguments for formatter creation.");
    return TT_FORMATTER_STATUS_INVALID_ARGUMENT;
  }

  try {
    const TtDayTexConfigV1* config_view = nullptr;
    std::string config_error;
    if (!formatter_c_config_bridge_v1::GetDayTexConfigView(
            formatter_config, &config_view, &config_error)) {
      SetLastError(TT_FORMATTER_STATUS_CONFIG_ERROR, config_error);
      *formatter_out = nullptr;
      return TT_FORMATTER_STATUS_CONFIG_ERROR;
    }
    auto tex_config = std::make_shared<DayTexConfig>(*config_view);
    auto formatter = std::make_unique<DayTexFormatter>(tex_config);
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

auto FormatReportImpl(DayTexFormatter* formatter,
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

DayTexFormatter::DayTexFormatter(std::shared_ptr<DayTexConfig> config)
    : BaseTexFormatter(config) {}

auto DayTexFormatter::FormatReportFromView(
    const TtDailyReportDataV1& data_view) const -> std::string {
  DailyReportData data =
      day_report_view_utils::BuildDailyContentData(data_view);

  std::string output;
  output += GeneratePreamble();
  FormatHeaderContent(output, data);

  if (IsEmptyData(data)) {
    output += GetNoRecordsMsg();
    output += "\n";
    output += GeneratePostfix();
    return output;
  }

  FormatExtraContent(output, data);

  const int kTitleSize = config_->GetCategoryTitleFontSize();
  const int kLineHeightTenths = kTitleSize * 12;
  const int kLineHeightScale = 10;
  output += "{";
  output += "\\fontsize{";
  output += std::to_string(kTitleSize);
  output += "}{";
  output += std::to_string(kLineHeightTenths / kLineHeightScale);
  output += ".";
  output += std::to_string(kLineHeightTenths % kLineHeightScale);
  output += "}\\selectfont";
  output += "\\section*{";
  output += TexUtils::EscapeLatex(config_->GetProjectBreakdownLabel());
  output += "}";
  output += "}\n\n";

  output += TexUtils::FormatProjectTree(
      data_view.projectTreeNodes, data_view.projectTreeNodeCount,
      data_view.totalDuration, GetAvgDays(data),
      config_->GetCategoryTitleFontSize(), config_->GetListTopSepPt(),
      config_->GetListItemSepEx());
  output += GeneratePostfix();
  return output;
}

auto DayTexFormatter::IsEmptyData(const DailyReportData& data) const -> bool {
  return data.total_duration == 0;
}

auto DayTexFormatter::GetAvgDays(const DailyReportData& /*data*/) const -> int {
  return 1;
}

auto DayTexFormatter::GetNoRecordsMsg() const -> std::string {
  return config_->GetNoRecords();
}

auto DayTexFormatter::GetKeywordColors() const
    -> std::map<std::string, std::string> {
  return config_->GetKeywordColors();
}

void DayTexFormatter::FormatHeaderContent(std::string& report_stream,
                                          const DailyReportData& data) const {
  DayTexUtils::DisplayHeader(report_stream, data, config_);
}

void DayTexFormatter::FormatExtraContent(std::string& report_stream,
                                         const DailyReportData& data) const {
  auto strategy = std::make_unique<LatexStrategy>(config_);
  StatFormatter stats_formatter(std::move(strategy));
  report_stream += stats_formatter.Format(data, config_);
  DayTexUtils::DisplayDetailedActivities(report_stream, data, config_);
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

  DayTexFormatter* formatter = nullptr;
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
    std::unique_ptr<DayTexFormatter>{static_cast<DayTexFormatter*>(handle)};
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
          kSupportedReportDataKinds.size(), "day latex", nullptr,
          &native_report_data, &validation_error_code,
          &validation_error_message)) {
    SetLastError(validation_error_code, validation_error_message);
    return validation_error_code;
  }

  auto* formatter = static_cast<DayTexFormatter*>(handle);
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
