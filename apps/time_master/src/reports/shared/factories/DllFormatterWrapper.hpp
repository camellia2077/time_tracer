// reports/shared/factories/DllFormatterWrapper.hpp
#ifndef DLL_FORMATTER_WRAPPER_HPP
#define DLL_FORMATTER_WRAPPER_HPP

#include "reports/shared/interfaces/IReportFormatter.hpp"
#include "common/AppConfig.hpp"
#include "reports/shared/data/DailyReportData.hpp"
#include <string>
#include <memory>
#include <stdexcept>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

// 这是一个模板类，专门用于包装从DLL加载的格式化器
template<typename ReportDataType>
class DllFormatterWrapper : public IReportFormatter<ReportDataType> {
public:
    // 构造函数负责加载DLL并获取函数指针
    DllFormatterWrapper(const std::string& dll_path, const AppConfig& config) {
#ifdef _WIN32
        dll_handle_ = LoadLibraryA(dll_path.c_str());
        if (!dll_handle_) {
            throw std::runtime_error("Failed to load DLL: " + dll_path + " (Error: " + std::to_string(GetLastError()) + ")");
        }

        // ==================== [核心修改] ====================
        // 使用 pragma 指令临时禁用 -Wcast-function-type 警告
        // 这是处理 GetProcAddress 和 C++ 强类型转换的标准做法
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wcast-function-type"
        create_func_ = (CreateFormatterFunc)GetProcAddress(dll_handle_, "create_formatter");
        destroy_func_ = (DestroyFormatterFunc)GetProcAddress(dll_handle_, "destroy_formatter");
        format_func_ = (FormatReportFunc)GetProcAddress(dll_handle_, "format_report");
        #pragma GCC diagnostic pop
        // ====================================================

#else
        // Linux/macOS a dynamic library
        dll_handle_ = dlopen(dll_path.c_str(), RTLD_LAZY);
        if (!dll_handle_) {
            throw std::runtime_error("Failed to load shared library: " + dll_path + " (" + dlerror() + ")");
        }
        create_func_ = (CreateFormatterFunc)dlsym(dll_handle_, "create_formatter");
        destroy_func_ = (DestroyFormatterFunc)dlsym(dll_handle_, "destroy_formatter");
        format_func_ = (FormatReportFunc)dlsym(dll_handle_, "format_report");
#endif

        if (!create_func_ || !destroy_func_ || !format_func_) {
            throw std::runtime_error("Failed to get function pointers from DLL: " + dll_path);
        }

        // 使用从DLL获取的函数创建格式化器实例
        formatter_handle_ = create_func_(config);
        if (!formatter_handle_) {
            throw std::runtime_error("create_formatter from DLL returned null.");
        }
    }

    // 析构函数负责销毁格式化器实例并卸载DLL
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

    // 实现 IReportFormatter 接口
    std::string format_report(const ReportDataType& data) const override {
        if (formatter_handle_ && format_func_) {
            // 调用DLL中的 format_report 函数
            const char* result_cstr = format_func_(formatter_handle_, data);
            return (result_cstr) ? std::string(result_cstr) : "";
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
    FormatReportFunc format_func_ = nullptr;
};

#endif // DLL_FORMATTER_WRAPPER_HPP