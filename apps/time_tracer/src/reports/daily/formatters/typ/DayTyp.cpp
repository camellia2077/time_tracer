// reports/daily/formatters/typ/DayTyp.cpp

#include "DayTyp.hpp"
#include "DayTypUtils.hpp"
#include "reports/shared/formatters/base/ProjectTreeFormatter.hpp"

#include <format>
#include <string>
#include <memory>
#include "reports/daily/formatters/typ/DayTypConfig.hpp"
#include "reports/shared/data/DailyReportData.hpp"
#include "reports/daily/formatters/statistics/StatFormatter.hpp"
#include "reports/daily/formatters/statistics/TypstStrategy.hpp"
#include "common/AppConfig.hpp"

// [新增] 在匿名命名空间中定义 Typst 格式化策略
namespace {
    class TypstFormattingStrategy : public reporting::IFormattingStrategy {
    public:
        TypstFormattingStrategy(std::string font, int font_size)
            : m_font(std::move(font)), m_font_size(font_size) {}

        std::string format_category_header(
            const std::string& category_name,
            const std::string& formatted_duration,
            double percentage) const override
        {
            // Typst 语法: 使用 #text 设置字体，== 表示二级标题
            return std::format(R"(#text(font: "{}", size: {}pt)[== {}])",
                m_font,
                m_font_size,
                std::format("{}: {} ({:.1f}%)",
                    category_name,
                    formatted_duration,
                    percentage
                )
            ) + "\n";
        }

        std::string format_tree_node(
            const std::string& project_name,
            const std::string& formatted_duration,
            int indent_level) const override
        {
            // Typst 语法: + 表示列表项
            return std::string(indent_level * 2, ' ') + "+ " + project_name + ": " + formatted_duration + "\n";
        }

    private:
        std::string m_font;
        int m_font_size;
    };
} // namespace


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

    auto strategy = std::make_unique<TypstStrategy>(config_);
    StatFormatter stats_formatter(std::move(strategy));
    ss << stats_formatter.format(data, config_);

    DayTypUtils::display_detailed_activities(ss, data, config_);

    // 添加统领性的标题 "= Project Breakdown"
    ss << std::format(R"(#text(font: "{}", size: {}pt)[= {}])",
        config_->get_category_title_font(),
        config_->get_category_title_font_size(),
        config_->get_project_breakdown_label()
    ) << "\n\n";

    // [核心修改] 使用 ProjectTreeFormatter 替代旧的 TypUtils 调用
    // 1. 创建策略
    auto tree_strategy = std::make_unique<TypstFormattingStrategy>(
        config_->get_category_title_font(),
        config_->get_category_title_font_size()
    );

    // 2. 创建格式化器 (使用安全的迭代实现)
    reporting::ProjectTreeFormatter tree_formatter(std::move(tree_strategy));

    // 3. 生成报告
    ss << tree_formatter.format_project_tree(
        data.project_tree,
        data.total_duration,
        1 // avg_days for daily report is 1
    );

    return ss.str();
}

// ... (export functions unchanged)
extern "C" {
    __declspec(dllexport) FormatterHandle create_formatter(const AppConfig& cfg) {
        auto typ_config = std::make_shared<DayTypConfig>(cfg.day_typ_config_path);
        auto formatter = new DayTyp(typ_config);
        return static_cast<FormatterHandle>(formatter);
    }

    __declspec(dllexport) void destroy_formatter(FormatterHandle handle) {
        if (handle) {
            delete static_cast<DayTyp*>(handle);
        }
    }

    static std::string report_buffer;

    __declspec(dllexport) const char* format_report(FormatterHandle handle, const DailyReportData& data) {
        if (handle) {
            auto* formatter = static_cast<DayTyp*>(handle);
            report_buffer = formatter->format_report(data);
            return report_buffer.c_str();
        }
        return "";
    }
}