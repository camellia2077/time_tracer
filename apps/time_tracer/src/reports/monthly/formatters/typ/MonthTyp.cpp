// reports/monthly/formatters/typ/MonthTyp.cpp
#include "MonthTyp.hpp"
#include <iomanip>
#include <format>
#include <vector>
#include <algorithm>
#include <memory> 
#include "reports/shared/utils/format/TimeFormat.hpp"

#include "reports/shared/formatters/base/ProjectTreeFormatter.hpp"

#include "reports/shared/factories/GenericFormatterFactory.hpp" 
#include "reports/monthly/formatters/typ/MonthTypConfig.hpp"   
#include "reports/shared/data/MonthlyReportData.hpp"
#include "common/AppConfig.hpp"

// [核心修改] 移除了静态注册逻辑

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
            // Typst: 使用 #text 设置字体，== 表示二级标题 (因为 Project Breakdown 是一级)
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
            // Typst: + 表示列表项
            return std::string(indent_level * 2, ' ') + "+ " + project_name + ": " + formatted_duration + "\n";
        }

    private:
        std::string m_font;
        int m_font_size;
    };
} // namespace

MonthTyp::MonthTyp(std::shared_ptr<MonthTypConfig> config) : config_(config) {} 

std::string MonthTyp::format_report(const MonthlyReportData& data) const {
    std::stringstream ss;

    ss << std::format(R"(#set page(margin: (top: {}cm, bottom: {}cm, left: {}cm, right: {}cm)))",
        config_->get_margin_top_cm(),
        config_->get_margin_bottom_cm(),
        config_->get_margin_left_cm(),
        config_->get_margin_right_cm()
    ) << "\n";
    
    std::string spacing_str = std::to_string(config_->get_line_spacing_em()) + "em";
    ss << std::format(R"(#set text(font: "{}", size: {}pt, spacing: {}))", 
        config_->get_base_font(),
        config_->get_base_font_size(),
        spacing_str
    ) << "\n\n";

    if (data.year_month == "INVALID") {
        ss << config_->get_invalid_format_message() << "\n";
        return ss.str();
    }

    _display_summary(ss, data);

    if (data.actual_days == 0) {
        ss << config_->get_no_records_message() << "\n";
        return ss.str();
    }

    _display_project_breakdown(ss, data);
    return ss.str();
}

void MonthTyp::_display_summary(std::stringstream& ss, const MonthlyReportData& data) const {
    // [修正] 移除 {}-{} 的拼接，直接放一个 {} 给 data.year_month
    std::string title = std::format(
        R"(#text(font: "{}", size: {}pt)[= {} {}])",
        config_->get_title_font(),
        config_->get_report_title_font_size(),
        config_->get_report_title(),
        data.year_month // 直接传入 "2025-01"
    );
    ss << title << "\n\n";

    if (data.actual_days > 0) {
        ss << std::format("+ *{}:* {}\n", config_->get_actual_days_label(), data.actual_days);
        ss << std::format("+ *{}:* {}\n", config_->get_total_time_label(), time_format_duration(data.total_duration, data.actual_days));
    }
}

void MonthTyp::_display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data) const {
    // 统领性的一级标题 (=)
    ss << std::format(R"(#text(font: "{}", size: {}pt)[= {}])",
        config_->get_category_title_font(),
        config_->get_category_title_font_size(),
        config_->get_project_breakdown_label()
    ) << "\n\n";

    // [核心修改] 使用 ProjectTreeFormatter 替代 TypUtils

    // 1. 创建策略
    auto strategy = std::make_unique<TypstFormattingStrategy>(
        config_->get_category_title_font(),
        config_->get_category_title_font_size()
    );

    // 2. 创建格式化器
    reporting::ProjectTreeFormatter formatter(std::move(strategy));

    // 3. 生成报告 (注意传入 actual_days 用于计算平均值)
    ss << formatter.format_project_tree(
        data.project_tree,
        data.total_duration,
        data.actual_days
    );
}

// C-style functions to be exported from the DLL
extern "C" {
    __declspec(dllexport) FormatterHandle create_formatter(const AppConfig& cfg) {
        auto typ_config = std::make_shared<MonthTypConfig>(cfg.month_typ_config_path);
        auto formatter = new MonthTyp(typ_config);
        return static_cast<FormatterHandle>(formatter);
    }

    __declspec(dllexport) void destroy_formatter(FormatterHandle handle) {
        if (handle) {
            delete static_cast<MonthTyp*>(handle);
        }
    }

    static std::string report_buffer;

    __declspec(dllexport) const char* format_report(FormatterHandle handle, const MonthlyReportData& data) {
        if (handle) {
            auto* formatter = static_cast<MonthTyp*>(handle);
            report_buffer = formatter->format_report(data);
            return report_buffer.c_str();
        }
        return "";
    }
}