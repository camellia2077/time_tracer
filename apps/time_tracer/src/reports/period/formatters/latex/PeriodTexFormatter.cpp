// reports/period/formatters/latex/PeriodTexFormatter.cpp
#include "PeriodTexFormatter.hpp"
#include <format>
#include "reports/period/formatters/latex/PeriodTexUtils.hpp" // [关键] 引入工具类声明，而不是实现
#include "common/AppConfig.hpp"

// 构造函数
PeriodTexFormatter::PeriodTexFormatter(std::shared_ptr<PeriodTexConfig> config) 
    : BaseTexFormatter(config) {}

std::string PeriodTexFormatter::validate_data(const PeriodReportData& data) const {
    if (data.days_to_query <= 0) {
        return config_->get_invalid_days_message();
    }
    return "";
}

bool PeriodTexFormatter::is_empty_data(const PeriodReportData& data) const {
    return data.actual_days == 0;
}

int PeriodTexFormatter::get_avg_days(const PeriodReportData& data) const {
    return data.actual_days;
}

std::string PeriodTexFormatter::get_no_records_msg() const {
    return config_->get_no_records_message();
}

void PeriodTexFormatter::format_header_content(std::stringstream& ss, const PeriodReportData& data) const {
    // [关键] 调用 PeriodTexUtils 中的实现，避免代码重复和链接错误
    PeriodTexUtils::display_summary(ss, data, config_);
}

// DLL 导出接口
extern "C" {
    __declspec(dllexport) FormatterHandle create_formatter(const AppConfig& cfg) {
        // 假设 AppConfig 中有 period_tex_config_path，参考了 Md/Typ 的命名规律
        auto tex_config = std::make_shared<PeriodTexConfig>(cfg.period_tex_config_path);
        auto formatter = new PeriodTexFormatter(tex_config);
        return static_cast<FormatterHandle>(formatter);
    }

    __declspec(dllexport) void destroy_formatter(FormatterHandle handle) {
        if (handle) {
            delete static_cast<PeriodTexFormatter*>(handle);
        }
    }

    static std::string report_buffer;

    __declspec(dllexport) const char* format_report(FormatterHandle handle, const PeriodReportData& data) {
        if (handle) {
            auto* formatter = static_cast<PeriodTexFormatter*>(handle);
            report_buffer = formatter->format_report(data);
            return report_buffer.c_str();
        }
        return "";
    }
}