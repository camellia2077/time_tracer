// queries/daily/formatters/typ/DayTypUtils.cpp
#include "DayTypUtils.hpp"
#include <iomanip>
#include <format>
#include <string>
#include <algorithm>
#include <vector>
#include "queries/shared/utils/format/BoolToString.hpp"
#include "queries/shared/utils/format/TimeFormat.hpp"
#include "queries/shared/utils/format/ReportStringUtils.hpp"

namespace { // 使用匿名命名空间来隐藏内部辅助函数

    std::string format_activity_line(const TimeRecord& record, const std::shared_ptr<DayTypConfig>& config) {
        std::string project_path = replace_all(record.project_path, "_", config->get_activity_connector());
        std::string base_string = std::format("{} - {} ({}): {}",
            record.start_time,
            record.end_time,
            time_format_duration(record.duration_seconds),
            project_path
        );

        for (const auto& pair : config->get_keyword_colors()) {
            if (record.project_path.find(pair.first) != std::string::npos) {
                const std::string& hex_color = pair.second;
                std::string typst_color_format = std::format(R"(rgb("{}"))", hex_color);
                std::string final_output = std::format("+ #text({})[{}]", typst_color_format, base_string);
                
                if (record.activityRemark.has_value()) {
                    final_output += std::format("\n  + *{}:* {}", config->get_activity_remark_label(), record.activityRemark.value());
                }
                return final_output;
            }
        }

        std::string final_output = "+ " + base_string;
        if (record.activityRemark.has_value()) {
            final_output += std::format("\n  + *{}:* {}", config->get_activity_remark_label(), record.activityRemark.value());
        }
        
        return final_output;
    }

    void generate_sorted_typ_output(std::stringstream& ss, const ProjectNode& node, int indent, int avg_days, const std::shared_ptr<DayTypConfig>& config) {
        std::vector<std::pair<std::string, ProjectNode>> sorted_children;
        for (const auto& pair : node.children) {
            sorted_children.push_back(pair);
        }
        std::sort(sorted_children.begin(), sorted_children.end(), [](const auto& a, const auto& b) {
            return a.second.duration > b.second.duration;
        });

        std::string indent_str(indent * 2, ' ');

        for (const auto& pair : sorted_children) {
            const std::string& name = pair.first;
            const ProjectNode& child_node = pair.second;

            if (child_node.duration > 0 || !child_node.children.empty()) {
                ss << indent_str << "+ " << name << ": " << time_format_duration(child_node.duration, avg_days) << "\n";
                generate_sorted_typ_output(ss, child_node, indent + 1, avg_days, config);
            }
        }
    }

    std::string format_project_tree(const ProjectTree& tree, long long total_duration, int avg_days, const std::shared_ptr<DayTypConfig>& config) {
        std::stringstream ss;
        std::vector<std::pair<std::string, ProjectNode>> sorted_top_level;
        for (const auto& pair : tree) {
            sorted_top_level.push_back(pair);
        }
        std::sort(sorted_top_level.begin(), sorted_top_level.end(), [](const auto& a, const auto& b) {
            return a.second.duration > b.second.duration;
        });

        for (const auto& pair : sorted_top_level) {
            const std::string& category_name = pair.first;
            const ProjectNode& category_node = pair.second;
            double percentage = (total_duration > 0) ? (static_cast<double>(category_node.duration) / total_duration * 100.0) : 0.0;

            ss << std::format(R"(#text(font: "{}", size: {}pt)[= {}])", 
                config->get_category_title_font(),
                config->get_category_title_font_size(),
                std::format("{}: {} ({:.1f}%)", 
                    category_name, 
                    time_format_duration(category_node.duration, avg_days), 
                    percentage
                )
            ) << "\n";

            generate_sorted_typ_output(ss, category_node, 0, avg_days, config);
        }

        return ss.str();
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
        ss << std::format("+ *{}:* {}\n", config->get_remark_label(), data.metadata.remark);
    }

    void display_statistics(std::stringstream& ss, const DailyReportData& data, const std::shared_ptr<DayTypConfig>& config) {
        ss << std::format(R"(#text(font: "{}", size: {}pt)[= {}])", 
            config->get_category_title_font(),
            config->get_category_title_font_size(), 
            config->get_statistics_label()) << "\n\n";
        ss << std::format("+ *{}:* {}\n", 
            config->get_sleep_time_label(), 
            time_format_duration(data.sleep_time)
        );
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

    void display_project_breakdown(std::stringstream& ss, const DailyReportData& data, const std::shared_ptr<DayTypConfig>& config) {
        ss << format_project_tree(data.project_tree, data.total_duration, 1, config);
    }

} // namespace DayTypUtils