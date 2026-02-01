// reports/daily/formatters/markdown/day_md_formatter.cpp
#include "day_md_formatter.hpp"
#include <iomanip>
#include <format>
#include <memory>
#include <string>
#include <toml++/toml.h>

#include "reports/shared/utils/format/bool_to_string.hpp"
#include "reports/shared/utils/format/time_format.hpp"
#include "reports/shared/utils/format/report_string_utils.hpp"

#include "reports/daily/formatters/statistics/stat_formatter.hpp"
#include "reports/daily/formatters/statistics/markdown_stat_strategy.hpp"

namespace {
constexpr int kRemarkIndent = 2;
const std::string kRemarkIndentPrefix = "  ";
constexpr const char* kEmptyReport = "";
}  // namespace

DayMdFormatter::DayMdFormatter(std::shared_ptr<DayMdConfig> config) 
    : BaseMdFormatter(config) {}

auto DayMdFormatter::is_empty_data(const DailyReportData& data) const -> bool {
    return data.total_duration == 0;
}

auto DayMdFormatter::get_avg_days(const DailyReportData& /*data*/) const -> int {
    return 1;
}

auto DayMdFormatter::get_no_records_msg() const -> std::string {
    return config_->get_no_records();
}

void DayMdFormatter::format_header_content(std::stringstream& report_stream,
                                           const DailyReportData& data) const {
    report_stream << std::format("## {0} {1}\n\n",
                                 config_->get_title_prefix(),
                                 data.date);
    report_stream << std::format("- **{0}**: {1}\n",
                                 config_->get_date_label(),
                                 data.date);
    report_stream << std::format("- **{0}**: {1}\n",
                                 config_->get_total_time_label(),
                                 time_format_duration(data.total_duration));
    report_stream << std::format("- **{0}**: {1}\n",
                                 config_->get_status_label(),
                                 bool_to_string(data.metadata.status));
    report_stream << std::format("- **{0}**: {1}\n",
                                 config_->get_sleep_label(),
                                 bool_to_string(data.metadata.sleep));
    report_stream << std::format("- **{0}**: {1}\n",
                                 config_->get_exercise_label(),
                                 bool_to_string(data.metadata.exercise));
    report_stream << std::format("- **{0}**: {1}\n",
                                 config_->get_getup_time_label(),
                                 data.metadata.getup_time);

    std::string formatted_remark = format_multiline_for_list(
        data.metadata.remark, kRemarkIndent, kRemarkIndentPrefix);
    report_stream << std::format("- **{0}**: {1}\n",
                                 config_->get_remark_label(),
                                 formatted_remark);
}

void DayMdFormatter::format_extra_content(std::stringstream& report_stream,
                                          const DailyReportData& data) const {
    auto strategy = std::make_unique<MarkdownStatStrategy>();
    StatFormatter stats_formatter(std::move(strategy));
    report_stream << stats_formatter.format(data, config_);
    _display_detailed_activities(report_stream, data);
}

void DayMdFormatter::_display_detailed_activities(
    std::stringstream& report_stream, const DailyReportData& data) const {
    if (!data.detailed_records.empty()) {
        report_stream << "\n## " << config_->get_all_activities_label() << "\n\n";
        for (const auto& record : data.detailed_records) {
            std::string project_path = replace_all(record.project_path, "_", config_->get_activity_connector());
            report_stream << std::format("- {0} - {1} ({2}): {3}\n",
                                         record.start_time,
                                         record.end_time,
                                         time_format_duration(record.duration_seconds),
                                         project_path);
            if (record.activityRemark.has_value()) {
                report_stream << std::format(
                    "  - **{0}**: {1}\n",
                    config_->get_activity_remark_label(),
                    record.activityRemark.value());
            }
        }
        report_stream << "\n";
    }
}

extern "C" {
    // [核心修改] 解析 TOML 字符串
    // Public API: keep symbol name stable for dynamic loading.
    // NOLINTNEXTLINE(readability-identifier-naming)
    __declspec(dllexport) auto create_formatter(const char* config_content)
        -> FormatterHandle {
        try {
            auto config_tbl = toml::parse(config_content);
            auto md_config = std::make_shared<DayMdConfig>(config_tbl);
            auto formatter = std::make_unique<DayMdFormatter>(md_config);
            return static_cast<FormatterHandle>(formatter.release());
        } catch (...) {
            return nullptr;
        }
    }

    // Public API: keep symbol name stable for dynamic loading.
    // NOLINTNEXTLINE(readability-identifier-naming)
    __declspec(dllexport) void destroy_formatter(FormatterHandle handle) {
        if (handle != nullptr) {
            std::unique_ptr<DayMdFormatter>{
                static_cast<DayMdFormatter*>(handle)};
        }
    }

    static std::string report_buffer;

    // Public API: keep symbol name stable for dynamic loading.
    // NOLINTNEXTLINE(readability-identifier-naming)
    __declspec(dllexport) auto format_report(FormatterHandle handle,
                                             const DailyReportData& data)
        -> const char* {
        if (handle != nullptr) {
            auto* formatter = static_cast<DayMdFormatter*>(handle);
            report_buffer = formatter->format_report(data);
            return report_buffer.c_str();
        }
        return kEmptyReport;
    }
}
