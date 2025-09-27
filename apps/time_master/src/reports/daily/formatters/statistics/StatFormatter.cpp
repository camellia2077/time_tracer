// reports/daily/formatters/statistics/StatFormatter.cpp
#include "StatFormatter.hpp"
#include "reports/shared/utils/format/TimeFormat.hpp"
#include <vector>

StatFormatter::StatFormatter(std::unique_ptr<IStatStrategy> strategy)
    : m_strategy(std::move(strategy)) {
    if (!m_strategy) {
        throw std::invalid_argument("Formatting strategy for StatFormatter cannot be null.");
    }
}

std::string StatFormatter::format(const DailyReportData& data, const std::shared_ptr<DayBaseConfig>& config) const {
    const auto& items_config = config->get_statistics_items();
    std::vector<std::string> lines_to_print;

    const std::vector<std::string> ordered_keys = {"sleep_time", "total_exercise_time", "grooming_time", "recreation_time"};

    for (const auto& key : ordered_keys) {
        auto it = items_config.find(key);
        if (it == items_config.end() || !it->second.show) continue;

        long long duration = 0;
        if (key == "sleep_time") duration = data.sleep_time;
        else if (key == "total_exercise_time") duration = data.total_exercise_time;
        else if (key == "grooming_time") duration = data.grooming_time;
        else if (key == "recreation_time") duration = data.recreation_time;

        lines_to_print.push_back(m_strategy->format_main_item(it->second.label, time_format_duration(duration)));

        // --- 嵌套逻辑的通用处理 ---
        if (key == "total_exercise_time") {
            if (items_config.count("anaerobic_time") && items_config.at("anaerobic_time").show) {
                lines_to_print.push_back(m_strategy->format_sub_item(items_config.at("anaerobic_time").label, time_format_duration(data.anaerobic_time)));
            }
            if (items_config.count("cardio_time") && items_config.at("cardio_time").show) {
                lines_to_print.push_back(m_strategy->format_sub_item(items_config.at("cardio_time").label, time_format_duration(data.cardio_time)));
            }
        }
        
        if (key == "recreation_time") {
            if (items_config.count("zhihu_time") && items_config.at("zhihu_time").show) {
                lines_to_print.push_back(m_strategy->format_sub_item(items_config.at("zhihu_time").label, time_format_duration(data.recreation_zhihu_time)));
            }
            if (items_config.count("bilibili_time") && items_config.at("bilibili_time").show) {
                lines_to_print.push_back(m_strategy->format_sub_item(items_config.at("bilibili_time").label, time_format_duration(data.recreation_bilibili_time)));
            }
            if (items_config.count("douyin_time") && items_config.at("douyin_time").show) {
                lines_to_print.push_back(m_strategy->format_sub_item(items_config.at("douyin_time").label, time_format_duration(data.recreation_douyin_time)));
            }
        }
    }

    if (lines_to_print.empty()) {
        return "";
    }

    std::string header = m_strategy->format_header(config->get_statistics_label());
    std::string body = m_strategy->build_output(lines_to_print);
    
    return header + body;
}