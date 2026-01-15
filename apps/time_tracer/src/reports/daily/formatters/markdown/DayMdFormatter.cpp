// reports/daily/formatters/markdown/DayMdFormatter.cpp
#include "DayMdFormatter.hpp"
#include <iomanip>
#include <format>
#include <memory>
#include <toml++/toml.h> // [修改] 引入 toml++

#include "reports/shared/utils/format/BoolToString.hpp"
#include "reports/shared/utils/format/TimeFormat.hpp"
#include "reports/shared/utils/format/ReportStringUtils.hpp"

#include "reports/daily/formatters/statistics/StatFormatter.hpp"
#include "reports/daily/formatters/statistics/MarkdownStatStrategy.hpp"

DayMdFormatter::DayMdFormatter(std::shared_ptr<DayMdConfig> config) 
    : BaseMdFormatter(config) {}

bool DayMdFormatter::is_empty_data(const DailyReportData& data) const {
    return data.total_duration == 0;
}

int DayMdFormatter::get_avg_days(const DailyReportData& /*data*/) const {
    return 1;
}

std::string DayMdFormatter::get_no_records_msg() const {
    return config_->get_no_records();
}

void DayMdFormatter::format_header_content(std::stringstream& ss, const DailyReportData& data) const {
    ss << std::format("## {0} {1}\n\n", config_->get_title_prefix(), data.date);
    ss << std::format("- **{0}**: {1}\n", config_->get_date_label(), data.date);
    ss << std::format("- **{0}**: {1}\n", config_->get_total_time_label(), time_format_duration(data.total_duration));
    ss << std::format("- **{0}**: {1}\n", config_->get_status_label(), bool_to_string(data.metadata.status));
    ss << std::format("- **{0}**: {1}\n", config_->get_sleep_label(), bool_to_string(data.metadata.sleep));
    ss << std::format("- **{0}**: {1}\n", config_->get_exercise_label(), bool_to_string(data.metadata.exercise));
    ss << std::format("- **{0}**: {1}\n", config_->get_getup_time_label(), data.metadata.getup_time);

    std::string formatted_remark = format_multiline_for_list(data.metadata.remark, 2, "  ");
    ss << std::format("- **{0}**: {1}\n", config_->get_remark_label(), formatted_remark);
}

void DayMdFormatter::format_extra_content(std::stringstream& ss, const DailyReportData& data) const {
    auto strategy = std::make_unique<MarkdownStatStrategy>();
    StatFormatter stats_formatter(std::move(strategy));
    ss << stats_formatter.format(data, config_);
    _display_detailed_activities(ss, data);
}

void DayMdFormatter::_display_detailed_activities(std::stringstream& ss, const DailyReportData& data) const {
    if (!data.detailed_records.empty()) {
        ss << "\n## " << config_->get_all_activities_label() << "\n\n";
        for (const auto& record : data.detailed_records) {
            std::string project_path = replace_all(record.project_path, "_", config_->get_activity_connector());
            ss << std::format("- {0} - {1} ({2}): {3}\n",
                record.start_time,
                record.end_time,
                time_format_duration(record.duration_seconds),
                project_path
            );
            if (record.activityRemark.has_value()) {
                ss << std::format("  - **{0}**: {1}\n", config_->get_activity_remark_label(), record.activityRemark.value());
            }
        }
        ss << "\n";
    }
}

extern "C" {
    // [核心修改] 解析 TOML 字符串
    __declspec(dllexport) FormatterHandle create_formatter(const char* config_content) {
        try {
            auto config_tbl = toml::parse(config_content);
            auto md_config = std::make_shared<DayMdConfig>(config_tbl);
            auto formatter = new DayMdFormatter(md_config);
            return static_cast<FormatterHandle>(formatter);
        } catch (...) {
            return nullptr;
        }
    }

    __declspec(dllexport) void destroy_formatter(FormatterHandle handle) {
        if (handle) {
            delete static_cast<DayMdFormatter*>(handle);
        }
    }

    static std::string report_buffer;

    __declspec(dllexport) const char* format_report(FormatterHandle handle, const DailyReportData& data) {
        if (handle) {
            auto* formatter = static_cast<DayMdFormatter*>(handle);
            report_buffer = formatter->format_report(data);
            return report_buffer.c_str();
        }
        return "";
    }
}