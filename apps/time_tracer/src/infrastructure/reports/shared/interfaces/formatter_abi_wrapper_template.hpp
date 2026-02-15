// infrastructure/reports/shared/interfaces/formatter_abi_wrapper_template.hpp
#ifndef INFRASTRUCTURE_REPORTS_SHARED_INTERFACES_FORMATTER_ABI_WRAPPER_TEMPLATE_H_
#define INFRASTRUCTURE_REPORTS_SHARED_INTERFACES_FORMATTER_ABI_WRAPPER_TEMPLATE_H_

#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <memory>
#include <string>

#include "infrastructure/reports/shared/interfaces/formatter_c_abi_v2.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_config_bridge_v1.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_report_data_bridge.hpp"

namespace infrastructure::reports::abi {

template <typename FormatterType, typename FormatterConfigType,
          typename ConfigViewType, typename ReportViewType>
struct FormatterAbiTraitsBase {
  using Formatter = FormatterType;
  using FormatterConfig = FormatterConfigType;
  using ConfigView = ConfigViewType;
  using ReportView = ReportViewType;

  static constexpr uint16_t kImplementationVersionMajor = 0U;
  static constexpr uint16_t kImplementationVersionMinor = 5U;
  static constexpr uint16_t kImplementationVersionPatch = 8U;
};

template <typename Derived, typename FormatterType,
          typename FormatterConfigType, typename ConfigViewType,
          auto GetConfigViewFn>
struct DailyFormatterAbiTraitsBase
    : FormatterAbiTraitsBase<FormatterType, FormatterConfigType, ConfigViewType,
                             TtDailyReportDataV1> {
  static auto GetConfigView(const TtFormatterConfig* formatter_config,
                            const ConfigViewType** out_config,
                            std::string* error_message) -> bool {
    return GetConfigViewFn(formatter_config, out_config, error_message);
  }

  static auto ValidateReportDataView(const void* report_data,
                                     const TtDailyReportDataV1** out_data,
                                     std::string* error_message) -> bool {
    return formatter_c_report_data_bridge::ValidateDailyReportDataView(
        report_data, out_data, error_message);
  }

  static constexpr std::array<uint32_t, 1> kSupportedReportDataKinds = {
      TT_REPORT_DATA_KIND_DAILY};
  static constexpr const char* kExpectedReportKindsHint = nullptr;
};

template <typename Derived, typename FormatterType,
          typename FormatterConfigType, typename ConfigViewType,
          auto GetConfigViewFn>
struct MonthlyFormatterAbiTraitsBase
    : FormatterAbiTraitsBase<FormatterType, FormatterConfigType, ConfigViewType,
                             TtRangeReportDataV1> {
  static auto GetConfigView(const TtFormatterConfig* formatter_config,
                            const ConfigViewType** out_config,
                            std::string* error_message) -> bool {
    return GetConfigViewFn(formatter_config, out_config, error_message);
  }

  static auto ValidateReportDataView(const void* report_data,
                                     const TtRangeReportDataV1** out_data,
                                     std::string* error_message) -> bool {
    return formatter_c_report_data_bridge::ValidateRangeReportDataView(
        report_data, out_data, error_message);
  }

  static constexpr std::array<uint32_t, 1> kSupportedReportDataKinds = {
      TT_REPORT_DATA_KIND_MONTHLY};
  static constexpr const char* kExpectedReportKindsHint = nullptr;
};

template <typename Derived, typename FormatterType,
          typename FormatterConfigType, typename ConfigViewType,
          auto GetConfigViewFn>
struct RangeFamilyFormatterAbiTraitsBase
    : FormatterAbiTraitsBase<FormatterType, FormatterConfigType, ConfigViewType,
                             TtRangeReportDataV1> {
  static auto GetConfigView(const TtFormatterConfig* formatter_config,
                            const ConfigViewType** out_config,
                            std::string* error_message) -> bool {
    return GetConfigViewFn(formatter_config, out_config, error_message);
  }

  static auto ValidateReportDataView(const void* report_data,
                                     const TtRangeReportDataV1** out_data,
                                     std::string* error_message) -> bool {
    return formatter_c_report_data_bridge::ValidateRangeReportDataView(
        report_data, out_data, error_message);
  }

  static constexpr std::array<uint32_t, 4> kSupportedReportDataKinds = {
      TT_REPORT_DATA_KIND_RANGE,
      TT_REPORT_DATA_KIND_PERIOD,
      TT_REPORT_DATA_KIND_WEEKLY,
      TT_REPORT_DATA_KIND_YEARLY,
  };
  static constexpr const char* kExpectedReportKindsHint =
      "Expected range/period/weekly/yearly.";
};

template <typename Traits>
class FormatterAbiWrapper {
 public:
  using FormatterType = typename Traits::Formatter;
  using FormatterConfigType = typename Traits::FormatterConfig;
  using ConfigViewType = typename Traits::ConfigView;
  using ReportViewType = typename Traits::ReportView;

  static auto GetFormatterAbiInfo(TtFormatterAbiInfo* out_abi) -> int32_t {
    if (out_abi == nullptr) {
      return TT_FORMATTER_STATUS_INVALID_ARGUMENT;
    }

    out_abi->structSize = static_cast<uint32_t>(sizeof(TtFormatterAbiInfo));
    out_abi->abiVersion = TT_FORMATTER_ABI_VERSION_CURRENT;
    out_abi->implementationVersion.major = Traits::kImplementationVersionMajor;
    out_abi->implementationVersion.minor = Traits::kImplementationVersionMinor;
    out_abi->implementationVersion.patch = Traits::kImplementationVersionPatch;
    out_abi->implementationVersion.reserved = 0U;
    return TT_FORMATTER_STATUS_OK;
  }

  static auto CreateFormatter(const TtFormatterConfig* config,
                              TtFormatterHandle* out_handle) -> int32_t {
    if (out_handle == nullptr) {
      SetLastError(TT_FORMATTER_STATUS_INVALID_ARGUMENT,
                   "out_handle must not be null.");
      return TT_FORMATTER_STATUS_INVALID_ARGUMENT;
    }

    FormatterType* formatter = nullptr;
    const auto status_code = CreateFormatterImpl(config, &formatter);
    if (status_code != TT_FORMATTER_STATUS_OK) {
      *out_handle = nullptr;
      return status_code;
    }

    *out_handle = static_cast<TtFormatterHandle>(formatter);
    return TT_FORMATTER_STATUS_OK;
  }

  static auto DestroyFormatter(TtFormatterHandle handle) -> int32_t {
    if (handle != nullptr) {
      std::unique_ptr<FormatterType>{static_cast<FormatterType*>(handle)};
    }
    ClearLastError();
    return TT_FORMATTER_STATUS_OK;
  }

  static auto FormatReport(TtFormatterHandle handle, const void* report_data,
                           uint32_t report_data_kind, char** out_report_content,
                           uint64_t* out_report_size) -> int32_t {
    if ((out_report_content == nullptr) || (out_report_size == nullptr)) {
      SetLastError(TT_FORMATTER_STATUS_INVALID_ARGUMENT,
                   "Output pointers must not be null.");
      return TT_FORMATTER_STATUS_INVALID_ARGUMENT;
    }
    *out_report_content = nullptr;
    *out_report_size = 0U;

    if ((handle == nullptr) || (report_data == nullptr)) {
      SetLastError(TT_FORMATTER_STATUS_INVALID_ARGUMENT,
                   "handle and report_data must not be null.");
      return TT_FORMATTER_STATUS_INVALID_ARGUMENT;
    }

    const void* native_report_data = nullptr;
    int32_t validation_error_code = TT_FORMATTER_STATUS_OK;
    std::string validation_error_message;
    if (!formatter_c_config_bridge_v1::ValidateReportDataView(
            report_data, report_data_kind,
            Traits::kSupportedReportDataKinds.data(),
            Traits::kSupportedReportDataKinds.size(), Traits::kFormatterName,
            Traits::kExpectedReportKindsHint, &native_report_data,
            &validation_error_code, &validation_error_message)) {
      SetLastError(validation_error_code, validation_error_message);
      return validation_error_code;
    }

    const ReportViewType* report_view = nullptr;
    if (!Traits::ValidateReportDataView(native_report_data, &report_view,
                                        &validation_error_message)) {
      SetLastError(TT_FORMATTER_STATUS_INVALID_ARGUMENT,
                   validation_error_message);
      return TT_FORMATTER_STATUS_INVALID_ARGUMENT;
    }

    auto* formatter = static_cast<FormatterType*>(handle);
    std::string formatted_report;
    const auto status_code =
        FormatReportImpl(formatter, report_view, &formatted_report);
    if (status_code != TT_FORMATTER_STATUS_OK) {
      return status_code;
    }

    const auto report_size = formatted_report.size();
    auto* copied_report =
        static_cast<char*>(std::malloc(report_size + static_cast<size_t>(1U)));
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

  static void FreeCString(char* c_string) {
    if (c_string != nullptr) {
      std::free(c_string);
    }
  }

  static auto GetLastError(TtFormatterHandle /*handle*/,
                           TtFormatterError* out_error) -> int32_t {
    if (out_error == nullptr) {
      return TT_FORMATTER_STATUS_INVALID_ARGUMENT;
    }

    out_error->structSize = static_cast<uint32_t>(sizeof(TtFormatterError));
    out_error->code = last_error_code_;
    out_error->message =
        last_error_message_.empty() ? nullptr : last_error_message_.c_str();
    return TT_FORMATTER_STATUS_OK;
  }

 private:
  static void SetLastError(int32_t error_code,
                           const std::string& error_message) {
    last_error_code_ = error_code;
    last_error_message_ = error_message;
  }

  static void ClearLastError() {
    last_error_code_ = TT_FORMATTER_STATUS_OK;
    last_error_message_.clear();
  }

  static auto CreateFormatterImpl(const TtFormatterConfig* formatter_config,
                                  FormatterType** formatter_out)
      -> TtFormatterStatusCode {
    if ((formatter_config == nullptr) || (formatter_out == nullptr)) {
      SetLastError(TT_FORMATTER_STATUS_INVALID_ARGUMENT,
                   "Invalid arguments for formatter creation.");
      return TT_FORMATTER_STATUS_INVALID_ARGUMENT;
    }

    try {
      const ConfigViewType* config_view = nullptr;
      std::string config_error;
      if (!Traits::GetConfigView(formatter_config, &config_view,
                                 &config_error)) {
        SetLastError(TT_FORMATTER_STATUS_CONFIG_ERROR, config_error);
        *formatter_out = nullptr;
        return TT_FORMATTER_STATUS_CONFIG_ERROR;
      }

      auto formatter = std::make_unique<FormatterType>(
          std::make_shared<FormatterConfigType>(*config_view));
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

  static auto FormatReportImpl(FormatterType* formatter,
                               const ReportViewType* data_view,
                               std::string* formatted_report)
      -> TtFormatterStatusCode {
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

  inline static thread_local std::string last_error_message_;
  inline static thread_local int32_t last_error_code_ = TT_FORMATTER_STATUS_OK;
};

}  // namespace infrastructure::reports::abi

#define TT_DEFINE_FORMATTER_ABI_EXPORTS(TRAITS_TYPE)                       \
  extern "C" {                                                             \
  __declspec(dllexport) auto tt_getFormatterAbiInfo(                       \
      TtFormatterAbiInfo* out_abi) -> int32_t {                            \
    return infrastructure::reports::abi::FormatterAbiWrapper<              \
        TRAITS_TYPE>::GetFormatterAbiInfo(out_abi);                        \
  }                                                                        \
                                                                           \
  __declspec(dllexport) auto tt_createFormatter(                           \
      const TtFormatterConfig* config, TtFormatterHandle* out_handle)      \
      -> int32_t {                                                         \
    return infrastructure::reports::abi::FormatterAbiWrapper<              \
        TRAITS_TYPE>::CreateFormatter(config, out_handle);                 \
  }                                                                        \
                                                                           \
  __declspec(dllexport) auto tt_destroyFormatter(TtFormatterHandle handle) \
      -> int32_t {                                                         \
    return infrastructure::reports::abi::FormatterAbiWrapper<              \
        TRAITS_TYPE>::DestroyFormatter(handle);                            \
  }                                                                        \
                                                                           \
  __declspec(dllexport) auto tt_formatReport(TtFormatterHandle handle,     \
                                             const void* report_data,      \
                                             uint32_t report_data_kind,    \
                                             char** out_report_content,    \
                                             uint64_t* out_report_size)    \
      -> int32_t {                                                         \
    return infrastructure::reports::abi::FormatterAbiWrapper<              \
        TRAITS_TYPE>::FormatReport(handle, report_data, report_data_kind,  \
                                   out_report_content, out_report_size);   \
  }                                                                        \
                                                                           \
  __declspec(dllexport) void tt_freeCString(char* c_string) {              \
    infrastructure::reports::abi::FormatterAbiWrapper<                     \
        TRAITS_TYPE>::FreeCString(c_string);                               \
  }                                                                        \
                                                                           \
  __declspec(dllexport) auto tt_getLastError(TtFormatterHandle handle,     \
                                             TtFormatterError* out_error)  \
      -> int32_t {                                                         \
    return infrastructure::reports::abi::FormatterAbiWrapper<              \
        TRAITS_TYPE>::GetLastError(handle, out_error);                     \
  }                                                                        \
  }

#endif  // INFRASTRUCTURE_REPORTS_SHARED_INTERFACES_FORMATTER_ABI_WRAPPER_TEMPLATE_H_
