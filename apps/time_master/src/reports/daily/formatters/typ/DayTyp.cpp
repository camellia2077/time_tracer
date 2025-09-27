// reports/daily/formatters/typ/DayTyp.cpp
#include "DayTyp.hpp"
#include "DayTypUtils.hpp"
#include "reports/shared/utils/format/TypUtils.hpp"
#include <format>
#include <string>
#include "reports/shared/factories/GenericFormatterFactory.hpp"
#include "reports/daily/formatters/typ/DayTypConfig.hpp"
#include "reports/shared/data/DailyReportData.hpp"
#include "reports/daily/formatters/statistics/StatFormatter.hpp"
#include "reports/daily/formatters/statistics/TypstStrategy.hpp"

// 自我注册逻辑保持不变
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

    // [核心修改] 使用新的 StatFormatter
    auto strategy = std::make_unique<TypstStrategy>(config_);
    StatFormatter stats_formatter(std::move(strategy));
    ss << stats_formatter.format(data, config_);

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
