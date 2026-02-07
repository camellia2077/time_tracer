// infrastructure/reports/shared/factories/dll_formatter_wrapper.hpp
#ifndef REPORTS_SHARED_FACTORIES_DLL_FORMATTER_WRAPPER_H_
#define REPORTS_SHARED_FACTORIES_DLL_FORMATTER_WRAPPER_H_

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "infrastructure/reports/shared/interfaces/i_report_formatter.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

template <typename ReportDataType>
class DllFormatterWrapper : public IReportFormatter<ReportDataType> {
 public:
  // [核心修改] 构造函数现在接收 config_json 字符串
  DllFormatterWrapper(const std::string& dll_path,
                      const std::string& config_json) {
#ifdef _WIN32
    dll_handle_ = LoadLibraryA(dll_path.c_str());
    if (dll_handle_ == nullptr) {
      throw std::runtime_error("Failed to load DLL: " + dll_path + " (Error: " +
                               std::to_string(GetLastError()) + ")");
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
    create_func_ = (CreateFormatterFunc)GetProcAddress(
        dll_handle_, "CreateFormatter");  // NOLINT
    destroy_func_ = (DestroyFormatterFunc)GetProcAddress(
        dll_handle_, "DestroyFormatter");  // NOLINT

    if constexpr (std::is_same_v<ReportDataType, DailyReportData>) {
      format_func_day_ = (FormatReportFunc_Day)GetProcAddress(
          dll_handle_, "FormatReport");  // NOLINT
    } else {
      // For all other types (Monthly, Period, Weekly, Yearly), they use the
      // same RangeReportData structure interface or compatible signature.
      format_func_range_ = (FormatReportFunc_Range)GetProcAddress(
          dll_handle_, "FormatReport");  // NOLINT
    }

#pragma GCC diagnostic pop
#else
    dll_handle_ = dlopen(dll_path.c_str(), RTLD_LAZY);
    if (!dll_handle_) {
      throw std::runtime_error("Failed to load shared library: " + dll_path +
                               " (Error: " + dlerror() + ")");
    }

    create_func_ =
        (CreateFormatterFunc)dlsym(dll_handle_, "CreateFormatter");  // NOLINT
    destroy_func_ =
        (DestroyFormatterFunc)dlsym(dll_handle_, "DestroyFormatter");  // NOLINT

    if constexpr (std::is_same_v<ReportDataType, DailyReportData>) {
      format_func_day_ =
          (FormatReportFunc_Day)dlsym(dll_handle_, "FormatReport");  // NOLINT
    } else {
      format_func_range_ =
          (FormatReportFunc_Range)dlsym(dll_handle_, "FormatReport");  // NOLINT
    }
#endif

    bool format_func_loaded = false;
    if constexpr (std::is_same_v<ReportDataType, DailyReportData>) {
      format_func_loaded = (format_func_day_ != nullptr);
    } else {
      format_func_loaded = (format_func_range_ != nullptr);
    }

    if (create_func_ == nullptr || destroy_func_ == nullptr ||
        !format_func_loaded) {
      throw std::runtime_error("Failed to get function pointers from DLL: " +
                               dll_path);
    }

    // [核心修改] 传递 JSON 字符串的 C 风格指针
    formatter_handle_ = create_func_(config_json.c_str());
    if (formatter_handle_ == nullptr) {
      throw std::runtime_error("create_formatter from DLL returned null.");
    }
  }

  ~DllFormatterWrapper() override {
    if ((formatter_handle_ != nullptr) && (destroy_func_ != nullptr)) {
      destroy_func_(formatter_handle_);
    }
#ifdef _WIN32
    if (dll_handle_ != nullptr) {
      FreeLibrary(dll_handle_);
    }
#else
    if (dll_handle_ != nullptr) {
      dlclose(dll_handle_);
    }
#endif
  }

  [[nodiscard]] auto FormatReport(const ReportDataType& data) const
      -> std::string override {
    if (formatter_handle_ != nullptr) {
      if constexpr (std::is_same_v<ReportDataType, DailyReportData>) {
        if (format_func_day_ != nullptr) {
          const char* result_cstr = format_func_day_(formatter_handle_, data);
          return (result_cstr != nullptr) ? std::string(result_cstr) : "";
        }
      } else {
        if (format_func_range_ != nullptr) {
          const char* result_cstr = format_func_range_(formatter_handle_, data);
          return (result_cstr != nullptr) ? std::string(result_cstr) : "";
        }
      }
    }
    return "Error: Formatter handle or format function is null.";
  }

 private:
#ifdef _WIN32
  HINSTANCE dll_handle_ = nullptr;
#else
  void* dll_handle_ = nullptr;
#endif
  FormatterHandle formatter_handle_ = nullptr;
  CreateFormatterFunc create_func_ = nullptr;
  DestroyFormatterFunc destroy_func_ = nullptr;

  FormatReportFunc_Day format_func_day_ = nullptr;
  FormatReportFunc_Range format_func_range_ = nullptr;
};

#endif  // REPORTS_SHARED_FACTORIES_DLL_FORMATTER_WRAPPER_H_
