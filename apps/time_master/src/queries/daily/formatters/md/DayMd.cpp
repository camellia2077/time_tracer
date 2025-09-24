// queries/daily/formatters/md/DayMd.cpp
#include "DayMd.hpp"
#include <iomanip>
#include <format>
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include "queries/shared/utils/format/BoolToString.hpp"
#include "queries/shared/utils/format/TimeFormat.hpp"
#include "queries/shared/utils/format/ReportStringUtils.hpp"
#include "queries/shared/utils/format/MarkdownUtils.hpp"
#include "queries/shared/factories/GenericFormatterFactory.hpp"
#include "queries/daily/formatters/md/DayMdConfig.hpp"
#include "queries/shared/data/DailyReportData.hpp"

// [新增] 自我注册逻辑
namespace {
    struct DayMdRegister {
        DayMdRegister() {
            GenericFormatterFactory<DailyReportData>::regist(ReportFormat::Markdown,
                [](const AppConfig& cfg) -> std::unique_ptr<IReportFormatter<DailyReportData>> {
                    auto md_config = std::make_shared<DayMdConfig>(cfg.day_md_config_path);
                    return std::make_unique<DayMd>(md_config);
                });
        }
    };
    const DayMdRegister registrar;
}

DayMd::DayMd(std::shared_ptr<DayMdConfig> config) : config_(config) {}

std::string DayMd::format_report(const DailyReportData& data) const {
    std::stringstream ss;
    _display_header(ss, data);

    if (data.total_duration == 0) {
        ss << config_->get_no_records() << "\n";
        return ss.str();
    }

    _display_statistics(ss, data);
    _display_detailed_activities(ss, data);
    _display_project_breakdown(ss, data);
    return ss.str();
}

void DayMd::_display_header(std::stringstream& ss, const DailyReportData& data) const {
    ss << std::format("## {0} {1}\n\n", config_->get_title_prefix(), data.date);
    ss << std::format("- **{0}**: {1}\n", config_->get_date_label(), data.date);
    ss << std::format("- **{0}**: {1}\n", config_->get_total_time_label(), time_format_duration(data.total_duration));
    ss << std::format("- **{0}**: {1}\n", config_->get_status_label(), bool_to_string(data.metadata.status));
    ss << std::format("- **{0}**: {1}\n", config_->get_sleep_label(), bool_to_string(data.metadata.sleep));
    ss << std::format("- **{0}**: {1}\n", config_->get_exercise_label(), bool_to_string(data.metadata.exercise));
    ss << std::format("- **{0}**: {1}\n", config_->get_getup_time_label(), data.metadata.getup_time);
    ss << std::format("- **{0}**: {1}\n", config_->get_remark_label(), data.metadata.remark);
}

void DayMd::_display_project_breakdown(std::stringstream& ss, const DailyReportData& data) const {
    ss << MarkdownUtils::format_project_tree(data.project_tree, data.total_duration, 1);
}

void DayMd::_display_detailed_activities(std::stringstream& ss, const DailyReportData& data) const {
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

void DayMd::_display_statistics(std::stringstream& ss, const DailyReportData& data) const {
    const auto& items_config = config_->get_statistics_items();
    std::vector<std::string> lines_to_print;

    // [核心修改] 更新 ordered_keys 以包含 total_exercise_time
    const std::vector<std::string> ordered_keys = {"sleep_time", "total_exercise_time", "grooming_time", "recreation_time"};

    for (const auto& key : ordered_keys) {
        auto it = items_config.find(key);
        if (it == items_config.end() || !it->second.show) continue;

        long long duration = 0;
        if (key == "sleep_time") duration = data.sleep_time;
        else if (key == "total_exercise_time") duration = data.total_exercise_time;
        else if (key == "grooming_time") duration = data.grooming_time;
        else if (key == "recreation_time") duration = data.recreation_time;

        lines_to_print.push_back(std::format("- **{0}**: {1}", it->second.label, time_format_duration(duration)));

        // [核心修改] 新增对 total_exercise_time 子项的处理
        if (key == "total_exercise_time") {
            if (items_config.count("anaerobic_time") && items_config.at("anaerobic_time").show) {
                lines_to_print.push_back(std::format("  - **{0}**: {1}", items_config.at("anaerobic_time").label, time_format_duration(data.anaerobic_time)));
            }
            if (items_config.count("cardio_time") && items_config.at("cardio_time").show) {
                lines_to_print.push_back(std::format("  - **{0}**: {1}", items_config.at("cardio_time").label, time_format_duration(data.cardio_time)));
            }
        }

        // 处理娱乐时间的子项
        if (key == "recreation_time") {
            if (items_config.count("zhihu_time") && items_config.at("zhihu_time").show) {
                lines_to_print.push_back(std::format("  - **{0}**: {1}", items_config.at("zhihu_time").label, time_format_duration(data.recreation_zhihu_time)));
            }
            if (items_config.count("bilibili_time") && items_config.at("bilibili_time").show) {
                lines_to_print.push_back(std::format("  - **{0}**: {1}", items_config.at("bilibili_time").label, time_format_duration(data.recreation_bilibili_time)));
            }
            if (items_config.count("douyin_time") && items_config.at("douyin_time").show) {
                lines_to_print.push_back(std::format("  - **{0}**: {1}", items_config.at("douyin_time").label, time_format_duration(data.recreation_douyin_time)));
            }
        }
    }

    if (lines_to_print.empty()) {
        return;
    }

    ss << "\n## " << config_->get_statistics_label() << "\n\n";
    for(const auto& line : lines_to_print) {
        ss << line << "\n";
    }
}