// reports/daily/formatters/md/DayMd.cpp
#include "DayMd.hpp"
#include <iomanip>
#include <format>
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <memory>
#include "reports/shared/utils/format/BoolToString.hpp"
#include "reports/shared/utils/format/TimeFormat.hpp"
#include "reports/shared/utils/format/ReportStringUtils.hpp"

#include "reports/shared/formatters/base/ProjectTreeFormatter.hpp" 

#include "reports/daily/formatters/md/DayMdConfig.hpp"
#include "reports/shared/data/DailyReportData.hpp"
#include "reports/daily/formatters/statistics/StatFormatter.hpp"
#include "reports/daily/formatters/statistics/MarkdownStrategy.hpp"
#include "common/AppConfig.hpp"

// [新增] 在匿名命名空间中定义 Markdown 格式化策略
namespace {
    class MarkdownFormattingStrategy : public reporting::IFormattingStrategy {
    public:
        std::string format_category_header(
            const std::string& category_name,
            const std::string& formatted_duration,
            double percentage) const override
        {
            // 使用 std::format 生成 Markdown 三级标题 (因为外层已经是二级标题)
            // 输出示例: ### Category: 2h 30m (25.0%) ###
            return std::format("\n### {}: {} ({:.1f}%) ###\n", 
                category_name, 
                formatted_duration, 
                percentage);
        }

        std::string format_tree_node(
            const std::string& project_name,
            const std::string& formatted_duration,
            int indent_level) const override
        {
            // Markdown 列表缩进，通常每级2或4个空格
            std::string indent(indent_level * 2, ' ');
            return std::format("{}- {}: {}\n", indent, project_name, formatted_duration);
        }
    };
} // namespace

DayMd::DayMd(std::shared_ptr<DayMdConfig> config) : config_(config) {}

std::string DayMd::format_report(const DailyReportData& data) const {
    std::stringstream ss;
    _display_header(ss, data);

    if (data.total_duration == 0) {
        ss << config_->get_no_records() << "\n";
        return ss.str();
    }

    auto strategy = std::make_unique<MarkdownStrategy>();
    StatFormatter stats_formatter(std::move(strategy));
    ss << stats_formatter.format(data, config_);

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
    // 先输出一个统领性的二级标题 (##)
    ss << "\n## " << config_->get_project_breakdown_label() << "\n"; 
    
    // [核心修改] 使用 ProjectTreeFormatter 替代 MarkdownUtils
    
    // 1. 创建策略
    auto strategy = std::make_unique<MarkdownFormattingStrategy>();

    // 2. 创建格式化器 (使用安全的迭代实现)
    reporting::ProjectTreeFormatter formatter(std::move(strategy));

    // 3. 生成并追加报告内容
    ss << formatter.format_project_tree(data.project_tree, data.total_duration, 1);
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


// C-style functions to be exported from the DLL
extern "C" {
    __declspec(dllexport) FormatterHandle create_formatter(const AppConfig& cfg) {
        auto md_config = std::make_shared<DayMdConfig>(cfg.day_md_config_path);
        auto formatter = new DayMd(md_config);
        return static_cast<FormatterHandle>(formatter);
    }

    __declspec(dllexport) void destroy_formatter(FormatterHandle handle) {
        if (handle) {
            delete static_cast<DayMd*>(handle);
        }
    }

    static std::string report_buffer;

    __declspec(dllexport) const char* format_report(FormatterHandle handle, const DailyReportData& data) {
        if (handle) {
            auto* formatter = static_cast<DayMd*>(handle);
            report_buffer = formatter->format_report(data);
            return report_buffer.c_str();
        }
        return "";
    }
}