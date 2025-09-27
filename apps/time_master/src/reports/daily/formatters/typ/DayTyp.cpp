// reports/daily/formatters/typ/DayTyp.cpp
#include "DayTyp.hpp"
#include "DayTypUtils.hpp"
#include "reports/shared/utils/format/TypUtils.hpp"
#include <format>
#include <string>
#include "reports/shared/factories/GenericFormatterFactory.hpp" // [新增]
#include "reports/daily/formatters/typ/DayTypConfig.hpp"
#include "reports/shared/data/DailyReportData.hpp"
#include "reports/shared/utils/format/TimeFormat.hpp"

// [新增] 自我注册逻辑
namespace {
    struct DayTypRegister {
        DayTypRegister() {
            GenericFormatterFactory<DailyReportData>::regist(ReportFormat::Typ,
                [](const AppConfig& cfg) -> std::unique_ptr<IReportFormatter<DailyReportData>> {
                    auto typ_config = std::make_shared<DayTypConfig>(cfg.day_typ_config_path);
                    return std::make_unique<DayTyp>(typ_config);
                });
        }
    };
    const DayTypRegister registrar;
}


DayTyp::DayTyp(std::shared_ptr<DayTypConfig> config) : config_(config) {}

std::string DayTyp::format_report(const DailyReportData& data) const {
    std::stringstream ss;
    std::string spacing_str = std::to_string(config_->get_line_spacing_em()) + "em";
    ss << std::format(R"(#set text(font: "{}", size: {}pt, spacing: {}))",
        config_->get_base_font(),
        config_->get_base_font_size(),
        spacing_str) << "\n\n";

    DayTypUtils::display_header(ss, data, config_);

    if (data.total_duration == 0) {
        ss << config_->get_no_records() << "\n";
        return ss.str();
    }

    _display_statistics(ss, data);
    DayTypUtils::display_detailed_activities(ss, data, config_);

    ss << TypUtils::format_project_tree(
        data.project_tree,
        data.total_duration,
        1, // avg_days for daily report is 1
        config_->get_category_title_font(),
        config_->get_category_title_font_size()
    );

    return ss.str();
}

void DayTyp::_display_statistics(std::stringstream& ss, const DailyReportData& data) const {
    const auto& items_config = config_->get_statistics_items();
    std::vector<std::string> lines_to_print;
    
    // [核心修改] 更新 ordered_keys
    const std::vector<std::string> ordered_keys = {"sleep_time", "total_exercise_time", "grooming_time", "recreation_time"};

    for (const auto& key : ordered_keys) {
        auto it = items_config.find(key);
        if (it == items_config.end() || !it->second.show) continue;

        long long duration = 0;
        if (key == "sleep_time") duration = data.sleep_time;
        else if (key == "total_exercise_time") duration = data.total_exercise_time;
        else if (key == "grooming_time") duration = data.grooming_time;
        else if (key == "recreation_time") duration = data.recreation_time;

        lines_to_print.push_back(std::format("- *{0}*: {1}", it->second.label, time_format_duration(duration)));

        // [核心修改] 新增对 total_exercise_time 子项的处理
        if (key == "total_exercise_time") {
            if (items_config.count("anaerobic_time") && items_config.at("anaerobic_time").show) {
                lines_to_print.push_back(std::format("  - *{0}*: {1}", items_config.at("anaerobic_time").label, time_format_duration(data.anaerobic_time)));
            }
            if (items_config.count("cardio_time") && items_config.at("cardio_time").show) {
                lines_to_print.push_back(std::format("  - *{0}*: {1}", items_config.at("cardio_time").label, time_format_duration(data.cardio_time)));
            }
        }
        
        // 特殊处理娱乐时间的子项
        if (key == "recreation_time") {
            if (items_config.count("zhihu_time") && items_config.at("zhihu_time").show) {
                lines_to_print.push_back(std::format("  - *{0}*: {1}", items_config.at("zhihu_time").label, time_format_duration(data.recreation_zhihu_time)));
            }
            if (items_config.count("bilibili_time") && items_config.at("bilibili_time").show) {
                lines_to_print.push_back(std::format("  - *{0}*: {1}", items_config.at("bilibili_time").label, time_format_duration(data.recreation_bilibili_time)));
            }
            if (items_config.count("douyin_time") && items_config.at("douyin_time").show) {
                lines_to_print.push_back(std::format("  - *{0}*: {1}", items_config.at("douyin_time").label, time_format_duration(data.recreation_douyin_time)));
            }
        }
    }

    if (lines_to_print.empty()) {
        return;
    }

    ss << std::format("#let statistic_font_size = {}pt\n", config_->get_statistic_font_size());
    ss << std::format("#let statistic_title_font_size = {}pt\n", config_->get_statistic_title_font_size());
    ss << "#set text(size: statistic_font_size)\n";
    ss << std::format("#align(center)[#text(size: statistic_title_font_size)[={0}]]\n\n", config_->get_statistics_label());

    for(const auto& line : lines_to_print) {
        ss << line << "\n";
    }
}