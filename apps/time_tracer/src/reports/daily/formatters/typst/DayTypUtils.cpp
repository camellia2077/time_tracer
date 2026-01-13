// reports/daily/formatters/typst/DayTypUtils.cpp
#include "DayTypUtils.hpp"
#include <iomanip>
#include <format>
#include <string>
#include <algorithm>
#include <vector>
#include "reports/shared/utils/format/BoolToString.hpp"
#include "reports/shared/utils/format/TimeFormat.hpp"
#include "reports/shared/utils/format/ReportStringUtils.hpp"

namespace { // 使用匿名命名空间来隐藏内部辅助函数

    std::string format_activity_line(const TimeRecord& record, const std::shared_ptr<DayTypConfig>& config) {
        std::string project_path = replace_all(record.project_path, "_", config->get_activity_connector());
        std::string base_string = std::format("{} - {} ({}): {}",
            record.start_time,
            record.end_time,
            time_format_duration(record.duration_seconds),
            project_path
        );

        // 1. 尝试匹配颜色关键字
        for (const auto& pair : config->get_keyword_colors()) {
            if (record.project_path.find(pair.first) != std::string::npos) {
                const std::string& hex_color = pair.second;
                std::string typst_color_format = std::format(R"(rgb("{}"))", hex_color);
                std::string final_output = std::format("+ #text({})[{}]", typst_color_format, base_string);
                
                if (record.activityRemark.has_value()) {
                    // [核心修改] 传入 " \\" 作为后缀，强制 Typst 换行
                    std::string formatted_activity_remark = format_multiline_for_list(
                        record.activityRemark.value(), 
                        4,     // 缩进 4 个空格
                        " \\"  // 行尾追加 " \"
                    );
                    final_output += std::format("\n  + *{}:* {}", config->get_activity_remark_label(), formatted_activity_remark);
                }
                return final_output;
            }
        }

        // 2. 默认格式（无颜色）
        std::string final_output = "+ " + base_string;
        if (record.activityRemark.has_value()) {
            // [核心修改] 传入 " \\" 作为后缀，强制 Typst 换行
            std::string formatted_activity_remark = format_multiline_for_list(
                record.activityRemark.value(), 
                4,     // 缩进 4 个空格
                " \\"  // 行尾追加 " \"
            );
            final_output += std::format("\n  + *{}:* {}", config->get_activity_remark_label(), formatted_activity_remark);
        }
        
        return final_output;
    }

} // 匿名命名空间结束

namespace DayTypUtils {

    void display_header(std::stringstream& ss, const DailyReportData& data, const std::shared_ptr<DayTypConfig>& config) {
        std::string title = std::format(
            R"(#text(font: "{}", size: {}pt)[= {} {}])",
            config->get_title_font(),
            config->get_report_title_font_size(),
            config->get_title_prefix(),
            data.date
        );
        ss << title << "\n\n";
        ss << std::format("+ *{}:* {}\n", config->get_date_label(), data.date);
        ss << std::format("+ *{}:* {}\n", config->get_total_time_label(), time_format_duration(data.total_duration));
        ss << std::format("+ *{}:* {}\n", config->get_status_label(), bool_to_string(data.metadata.status));
        ss << std::format("+ *{}:* {}\n", config->get_sleep_label(), bool_to_string(data.metadata.sleep));
        ss << std::format("+ *{}:* {}\n", config->get_exercise_label(), bool_to_string(data.metadata.exercise));
        ss << std::format("+ *{}:* {}\n", config->get_getup_time_label(), data.metadata.getup_time);

        // [核心修改] 传入 " \\" 作为后缀，强制 Typst 换行
        // 缩进 2 个空格以适配一级列表
        std::string formatted_remark = format_multiline_for_list(
            data.metadata.remark, 
            2, 
            " \\"
        );
        ss << std::format("+ *{}:* {}\n", config->get_remark_label(), formatted_remark);
    }

    void display_detailed_activities(std::stringstream& ss, const DailyReportData& data, const std::shared_ptr<DayTypConfig>& config) {
        if (!data.detailed_records.empty()) {
            ss << std::format(R"(#text(font: "{}", size: {}pt)[= {}])", 
                config->get_category_title_font(),
                config->get_category_title_font_size(), 
                config->get_all_activities_label()) << "\n\n";
            for (const auto& record : data.detailed_records) {
                ss << format_activity_line(record, config) << "\n";
            }
        }
    }

} // namespace DayTypUtils