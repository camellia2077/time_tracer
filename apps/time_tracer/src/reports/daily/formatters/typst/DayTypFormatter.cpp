// reports/daily/formatters/typst/DayTypFormatter.cpp
#include "DayTypFormatter.hpp"
#include "DayTypUtils.hpp"

#include "reports/daily/formatters/statistics/StatFormatter.hpp"
#include "reports/daily/formatters/statistics/TypstStrategy.hpp"
#include "common/AppConfig.hpp"
#include <memory>

DayTypFormatter::DayTypFormatter(std::shared_ptr<DayTypConfig> config) 
    : BaseTypFormatter(config) {}

bool DayTypFormatter::is_empty_data(const DailyReportData& data) const {
    return data.total_duration == 0;
}

// [修改] 注释掉未使用参数
int DayTypFormatter::get_avg_days(const DailyReportData& /*data*/) const {
    return 1;
}

std::string DayTypFormatter::get_no_records_msg() const {
    return config_->get_no_records();
}

void DayTypFormatter::format_header_content(std::stringstream& ss, const DailyReportData& data) const {
    DayTypUtils::display_header(ss, data, config_);
}

void DayTypFormatter::format_extra_content(std::stringstream& ss, const DailyReportData& data) const {
    // 1. 统计信息
    auto strategy = std::make_unique<TypstStrategy>(config_);
    StatFormatter stats_formatter(std::move(strategy));
    ss << stats_formatter.format(data, config_);

    // 2. 详细活动记录
    DayTypUtils::display_detailed_activities(ss, data, config_);
}

// DLL 导出部分保持不变
extern "C" {
    __declspec(dllexport) FormatterHandle create_formatter(const AppConfig& cfg) {
        auto typ_config = std::make_shared<DayTypConfig>(cfg.day_typ_config_path);
        auto formatter = new DayTypFormatter(typ_config);
        return static_cast<FormatterHandle>(formatter);
    }

    __declspec(dllexport) void destroy_formatter(FormatterHandle handle) {
        if (handle) {
            delete static_cast<DayTypFormatter*>(handle);
        }
    }

    static std::string report_buffer;

    __declspec(dllexport) const char* format_report(FormatterHandle handle, const DailyReportData& data) {
        if (handle) {
            auto* formatter = static_cast<DayTypFormatter*>(handle);
            report_buffer = formatter->format_report(data);
            return report_buffer.c_str();
        }
        return "";
    }
}