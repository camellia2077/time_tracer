// reports/shared/factories/DllFormatterWrapper.hpp
#ifndef DLL_FORMATTER_WRAPPER_HPP
#define DLL_FORMATTER_WRAPPER_HPP

#include "reports/shared/interfaces/IReportFormatter.hpp"
#include "common/AppConfig.hpp"
#include "reports/shared/data/DailyReportData.hpp"
#include "reports/shared/data/MonthlyReportData.hpp"
#include "reports/shared/data/PeriodReportData.hpp"
#include <string>
#include <memory>
#include <stdexcept>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h> // [新增] 引入 POSIX dlfcn.h 头文件
#endif

template<typename ReportDataType>
class DllFormatterWrapper : public IReportFormatter<ReportDataType> {
public:
    DllFormatterWrapper(const std::string& dll_path, const AppConfig& config) {
#ifdef _WIN32
        dll_handle_ = LoadLibraryA(dll_path.c_str());
        if (!dll_handle_) {
            throw std::runtime_error("Failed to load DLL: " + dll_path + " (Error: " + std::to_string(GetLastError()) + ")");
        }

        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wcast-function-type"
        create_func_ = (CreateFormatterFunc)GetProcAddress(dll_handle_, "create_formatter");
        destroy_func_ = (DestroyFormatterFunc)GetProcAddress(dll_handle_, "destroy_formatter");

        if constexpr (std::is_same_v<ReportDataType, DailyReportData>) {
            format_func_day_ = (FormatReportFunc_Day)GetProcAddress(dll_handle_, "format_report");
        } else if constexpr (std::is_same_v<ReportDataType, MonthlyReportData>) {
            format_func_month_ = (FormatReportFunc_Month)GetProcAddress(dll_handle_, "format_report");
        } else if constexpr (std::is_same_v<ReportDataType, PeriodReportData>) {
            format_func_period_ = (FormatReportFunc_Period)GetProcAddress(dll_handle_, "format_report");
        }

        #pragma GCC diagnostic pop
#else
        dll_handle_ = dlopen(dll_path.c_str(), RTLD_LAZY);
        if (!dll_handle_) {
            throw std::runtime_error("Failed to load shared library: " + dll_path + " (Error: " + dlerror() + ")");
        }

        create_func_ = (CreateFormatterFunc)dlsym(dll_handle_, "create_formatter");
        destroy_func_ = (DestroyFormatterFunc)dlsym(dll_handle_, "destroy_formatter");

        if constexpr (std::is_same_v<ReportDataType, DailyReportData>) {
            format_func_day_ = (FormatReportFunc_Day)dlsym(dll_handle_, "format_report");
        } else if constexpr (std::is_same_v<ReportDataType, MonthlyReportData>) {
            format_func_month_ = (FormatReportFunc_Month)dlsym(dll_handle_, "format_report");
        } else if constexpr (std::is_same_v<ReportDataType, PeriodReportData>) {
            format_func_period_ = (FormatReportFunc_Period)dlsym(dll_handle_, "format_report");
        }
        // --- 实现结束 ---
#endif
        bool format_func_loaded = false;
        if constexpr (std::is_same_v<ReportDataType, DailyReportData>) {
            format_func_loaded = (format_func_day_ != nullptr);
        } else if constexpr (std::is_same_v<ReportDataType, MonthlyReportData>) {
            format_func_loaded = (format_func_month_ != nullptr);
        } else if constexpr (std::is_same_v<ReportDataType, PeriodReportData>) {
            format_func_loaded = (format_func_period_ != nullptr);
        }

        if (!create_func_ || !destroy_func_ || !format_func_loaded) {
            throw std::runtime_error("Failed to get function pointers from DLL: " + dll_path);
        }

        formatter_handle_ = create_func_(config);
        if (!formatter_handle_) {
            throw std::runtime_error("create_formatter from DLL returned null.");
        }
    }

    ~DllFormatterWrapper() override {
        if (formatter_handle_ && destroy_func_) {
            destroy_func_(formatter_handle_);
        }
#ifdef _WIN32
        if (dll_handle_) {
            FreeLibrary(dll_handle_);
        }
#else
        if (dll_handle_) {
            dlclose(dll_handle_);
        }
#endif
    }

    std::string format_report(const ReportDataType& data) const override {
        if (formatter_handle_) {
            if constexpr (std::is_same_v<ReportDataType, DailyReportData>) {
                if (format_func_day_) {
                    const char* result_cstr = format_func_day_(formatter_handle_, data);
                    return (result_cstr) ? std::string(result_cstr) : "";
                }
            } else if constexpr (std::is_same_v<ReportDataType, MonthlyReportData>) {
                if (format_func_month_) {
                    const char* result_cstr = format_func_month_(formatter_handle_, data);
                    return (result_cstr) ? std::string(result_cstr) : "";
                }
            } else if constexpr (std::is_same_v<ReportDataType, PeriodReportData>) {
                if (format_func_period_) {
                    const char* result_cstr = format_func_period_(formatter_handle_, data);
                    return (result_cstr) ? std::string(result_cstr) : "";
                }
            }
        }
        return "Error: Formatter handle or format function is null.";
    }

private:
#ifdef _WIN32
    HINSTANCE dll_handle_ = nullptr;
#else
    void* dll_handle_ = nullptr; // [修改] POSIX API 使用 void* 作为句柄
#endif
    FormatterHandle formatter_handle_ = nullptr;
    CreateFormatterFunc create_func_ = nullptr;
    DestroyFormatterFunc destroy_func_ = nullptr;
    
    FormatReportFunc_Day format_func_day_ = nullptr;
    FormatReportFunc_Month format_func_month_ = nullptr;
    FormatReportFunc_Period format_func_period_ = nullptr;
};

#endif // DLL_FORMATTER_WRAPPER_HPP