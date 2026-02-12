// infrastructure/reports/shared/factories/dll_formatter_wrapper.hpp
#ifndef REPORTS_SHARED_FACTORIES_DLL_FORMATTER_WRAPPER_H_
#define REPORTS_SHARED_FACTORIES_DLL_FORMATTER_WRAPPER_H_

#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include "infrastructure/reports/shared/factories/formatter_config_payload.hpp"
#include "infrastructure/reports/shared/factories/report_data_payload_v1.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_abi_v2.hpp"
#include "infrastructure/reports/shared/interfaces/i_report_formatter.hpp"
#include "shared/types/exceptions.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

template <typename ReportDataType>
class DllFormatterWrapper : public IReportFormatter<ReportDataType> {
 public:
  // Keep FormatterConfigPayload as const reference:
  // its C ABI view stores string_view-style pointers into payload-owned
  // strings, and moving/copying by value here can invalidate those addresses.
  DllFormatterWrapper(std::string dll_path,
                      const FormatterConfigPayload& config_payload)
      : dll_path_(std::move(dll_path)) {
    OpenLibrary();
    LoadV2EntryPoints();
    InitializeFormatterWithV2(config_payload.GetCConfig());
  }

  ~DllFormatterWrapper() override {
    ReleaseFormatter();
    CloseLibrary();
  }

  [[nodiscard]] auto FormatReport(const ReportDataType& data) const
      -> std::string override {
    if (formatter_handle_ == nullptr) {
      return "Error: Formatter handle is null.";
    }
    return FormatReportWithV2(data);
  }

 private:
  static constexpr const char* kV2GetAbiInfoSymbol = "tt_getFormatterAbiInfo";
  static constexpr const char* kV2CreateSymbol = "tt_createFormatter";
  static constexpr const char* kV2DestroySymbol = "tt_destroyFormatter";
  static constexpr const char* kV2FormatSymbol = "tt_formatReport";
  static constexpr const char* kV2FreeCStringSymbol = "tt_freeCString";
  static constexpr const char* kV2GetLastErrorSymbol = "tt_getLastError";

  [[nodiscard]] auto FormatReportWithV2(const ReportDataType& data) const
      -> std::string {
    if (format_func_v2_ == nullptr) {
      throw time_tracer::common::DllCompatibilityError(
          "ABI v2 format function is null.");
    }

    auto report_payload =
        report_data_payload_v1::ReportDataPayloadV1::BuildFrom(data);
    TtReportDataView report_data_view{};
    report_data_view.structSize =
        static_cast<uint32_t>(sizeof(TtReportDataView));
    report_data_view.version = TT_REPORT_DATA_VIEW_VERSION_CURRENT;
    report_data_view.reportDataKind = GetReportDataKindForV2();
    report_data_view.reportDataVersion = TT_REPORT_DATA_VERSION_V1;
    report_data_view.reportData = report_payload.Data();
    report_data_view.reportDataSize = report_payload.DataSize();

    char* report_content = nullptr;
    uint64_t report_size = 0;
    auto status_code = static_cast<TtFormatterStatusCode>(format_func_v2_(
        formatter_handle_,
        static_cast<const void*>(std::addressof(report_data_view)),
        report_data_view.reportDataKind, &report_content, &report_size));
    if (status_code != TT_FORMATTER_STATUS_OK) {
      std::string error_message = BuildV2ErrorMessage(
          "Error formatting report through ABI v2", status_code);
      if (status_code == TT_FORMATTER_STATUS_NOT_SUPPORTED) {
        throw time_tracer::common::DllCompatibilityError(error_message);
      }
      throw time_tracer::common::LogicError(error_message);
    }

    if (report_content == nullptr) {
      return "";
    }

    std::string report_result;
    if (report_size > 0U) {
      report_result.assign(report_content,
                           static_cast<std::size_t>(report_size));
    } else {
      report_result = report_content;
    }

    if (free_c_string_func_v2_ != nullptr) {
      free_c_string_func_v2_(report_content);
    }
    return report_result;
  }

  [[nodiscard]] static auto GetReportDataKindForV2() -> uint32_t {
    if constexpr (std::is_same_v<ReportDataType, DailyReportData>) {
      return TT_REPORT_DATA_KIND_DAILY;
    }
    if constexpr (std::is_same_v<ReportDataType, MonthlyReportData>) {
      return TT_REPORT_DATA_KIND_MONTHLY;
    }
    if constexpr (std::is_same_v<ReportDataType, PeriodReportData>) {
      return TT_REPORT_DATA_KIND_PERIOD;
    }
    if constexpr (std::is_same_v<ReportDataType, RangeReportData>) {
      return TT_REPORT_DATA_KIND_RANGE;
    }
    if constexpr (std::is_same_v<ReportDataType, WeeklyReportData>) {
      return TT_REPORT_DATA_KIND_WEEKLY;
    }
    if constexpr (std::is_same_v<ReportDataType, YearlyReportData>) {
      return TT_REPORT_DATA_KIND_YEARLY;
    }
    return TT_REPORT_DATA_KIND_UNKNOWN;
  }

  void OpenLibrary() {
#ifdef _WIN32
    dll_handle_ = LoadLibraryA(dll_path_.c_str());
    if (dll_handle_ == nullptr) {
      throw std::runtime_error("Failed to load DLL: " + dll_path_ +
                               " (Error: " + std::to_string(GetLastError()) +
                               ")");
    }
#else
    dll_handle_ = dlopen(dll_path_.c_str(), RTLD_LAZY);
    if (dll_handle_ == nullptr) {
      throw std::runtime_error("Failed to load shared library: " + dll_path_ +
                               " (Error: " + dlerror() + ")");
    }
#endif
  }

  void CloseLibrary() {
#ifdef _WIN32
    if (dll_handle_ != nullptr) {
      FreeLibrary(dll_handle_);
      dll_handle_ = nullptr;
    }
#else
    if (dll_handle_ != nullptr) {
      dlclose(dll_handle_);
      dll_handle_ = nullptr;
    }
#endif
  }

  template <typename FuncType>
  [[nodiscard]] auto TryLoadSymbol(const char* symbol_name) const -> FuncType {
#ifdef _WIN32
    auto symbol = GetProcAddress(dll_handle_, symbol_name);
    return reinterpret_cast<FuncType>(symbol);
#else
    auto* symbol = dlsym(dll_handle_, symbol_name);
    return reinterpret_cast<FuncType>(symbol);
#endif
  }

  template <typename FuncType>
  void EnsureRequiredV2Symbol(FuncType loaded_symbol,
                              const char* symbol_name) const {
    if (loaded_symbol == nullptr) {
      throw time_tracer::common::DllCompatibilityError(
          "Missing required ABI v2 symbol '" + std::string(symbol_name) +
          "' in DLL: " + dll_path_ + ".");
    }
  }

  void LoadV2EntryPoints() {
    get_abi_info_func_v2_ =
        TryLoadSymbol<TtGetFormatterAbiInfoFuncV2>(kV2GetAbiInfoSymbol);
    create_func_v2_ = TryLoadSymbol<TtCreateFormatterFuncV2>(kV2CreateSymbol);
    destroy_func_v2_ =
        TryLoadSymbol<TtDestroyFormatterFuncV2>(kV2DestroySymbol);
    format_func_v2_ = TryLoadSymbol<TtFormatReportFuncV2>(kV2FormatSymbol);
    free_c_string_func_v2_ =
        TryLoadSymbol<TtFreeCStringFuncV2>(kV2FreeCStringSymbol);
    get_last_error_func_v2_ =
        TryLoadSymbol<TtGetLastErrorFuncV2>(kV2GetLastErrorSymbol);
    EnsureRequiredV2Symbol(create_func_v2_, kV2CreateSymbol);
    EnsureRequiredV2Symbol(destroy_func_v2_, kV2DestroySymbol);
    EnsureRequiredV2Symbol(format_func_v2_, kV2FormatSymbol);
    EnsureRequiredV2Symbol(free_c_string_func_v2_, kV2FreeCStringSymbol);
  }

  void InitializeFormatterWithV2(const TtFormatterConfig& formatter_config) {
    ValidateV2AbiInfo();

    TtFormatterHandle formatter_handle = nullptr;
    auto status_code = static_cast<TtFormatterStatusCode>(
        create_func_v2_(&formatter_config, &formatter_handle));
    if ((status_code != TT_FORMATTER_STATUS_OK) ||
        (formatter_handle == nullptr)) {
      std::string error_message =
          BuildV2ErrorMessage("tt_createFormatter failed", status_code);
      if (status_code == TT_FORMATTER_STATUS_NOT_SUPPORTED) {
        throw time_tracer::common::DllCompatibilityError(error_message);
      }
      throw time_tracer::common::LogicError(error_message);
    }
    formatter_handle_ = formatter_handle;
  }

  void ValidateV2AbiInfo() const {
    if (get_abi_info_func_v2_ == nullptr) {
      return;
    }

    TtFormatterAbiInfo abi_info{};
    abi_info.structSize = static_cast<uint32_t>(sizeof(TtFormatterAbiInfo));
    auto status_code =
        static_cast<TtFormatterStatusCode>(get_abi_info_func_v2_(&abi_info));
    if (status_code != TT_FORMATTER_STATUS_OK) {
      throw time_tracer::common::DllCompatibilityError(
          BuildV2ErrorMessage("tt_getFormatterAbiInfo failed", status_code));
    }

    if (abi_info.abiVersion != TT_FORMATTER_ABI_VERSION_CURRENT) {
      throw time_tracer::common::DllCompatibilityError(
          "ABI mismatch. Host expects version " +
          std::to_string(TT_FORMATTER_ABI_VERSION_CURRENT) +
          ", plugin provides " + std::to_string(abi_info.abiVersion) + ".");
    }
  }

  void ReleaseFormatter() {
    if (formatter_handle_ == nullptr) {
      return;
    }

    if (destroy_func_v2_ != nullptr) {
      static_cast<void>(destroy_func_v2_(formatter_handle_));
    }
    formatter_handle_ = nullptr;
  }

  [[nodiscard]] auto BuildV2ErrorMessage(
      const std::string& prefix, TtFormatterStatusCode status_code) const
      -> std::string {
    std::string error_message =
        prefix +
        " (status code: " + std::to_string(static_cast<int32_t>(status_code)) +
        ")";
    if ((get_last_error_func_v2_ != nullptr) &&
        (formatter_handle_ != nullptr)) {
      TtFormatterError formatter_error{};
      formatter_error.structSize =
          static_cast<uint32_t>(sizeof(TtFormatterError));
      auto error_status = static_cast<TtFormatterStatusCode>(
          get_last_error_func_v2_(formatter_handle_, &formatter_error));
      if ((error_status == TT_FORMATTER_STATUS_OK) &&
          (formatter_error.message != nullptr)) {
        error_message += ": ";
        error_message += formatter_error.message;
      }
    }
    return error_message;
  }

#ifdef _WIN32
  HINSTANCE dll_handle_ = nullptr;
#else
  void* dll_handle_ = nullptr;
#endif
  std::string dll_path_;

  TtFormatterHandle formatter_handle_ = nullptr;

  TtGetFormatterAbiInfoFuncV2 get_abi_info_func_v2_ = nullptr;
  TtCreateFormatterFuncV2 create_func_v2_ = nullptr;
  TtDestroyFormatterFuncV2 destroy_func_v2_ = nullptr;
  TtFormatReportFuncV2 format_func_v2_ = nullptr;
  TtFreeCStringFuncV2 free_c_string_func_v2_ = nullptr;
  TtGetLastErrorFuncV2 get_last_error_func_v2_ = nullptr;
};

#endif  // REPORTS_SHARED_FACTORIES_DLL_FORMATTER_WRAPPER_H_
