// queries/period/formatters/md/PeriodMd.cpp
#include "PeriodMd.hpp"
#include <iomanip>
#include <format>
#include <vector>
#include <algorithm>
#include "queries/shared/utils/format/TimeFormat.hpp"
#include "queries/shared/utils/format/MarkdownUtils.hpp"
#include "queries/shared/factories/GenericFormatterFactory.hpp" // [新增]
#include "queries/period/formatters/md/PeriodMdConfig.hpp"      // [新增]
#include "queries/shared/data/PeriodReportData.hpp"            // [新增]

// [新增] 自我注册逻辑
namespace {
    struct PeriodMdRegister {
        PeriodMdRegister() {
            GenericFormatterFactory<PeriodReportData>::regist(ReportFormat::Markdown, 
                [](const AppConfig& cfg) -> std::unique_ptr<IReportFormatter<PeriodReportData>> {
                    auto md_config = std::make_shared<PeriodMdConfig>(cfg.period_md_config_path);
                    return std::make_unique<PeriodMd>(md_config);
                });
        }
    };
    const PeriodMdRegister registrar;
}


PeriodMd::PeriodMd(std::shared_ptr<PeriodMdConfig> config) : config_(config) {}

std::string PeriodMd::format_report(const PeriodReportData& data) const {
    std::stringstream ss;
    if (data.days_to_query <= 0) {
        ss << config_->get_invalid_days_message() << "\n";
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

void PeriodMd::_display_summary(std::stringstream& ss, const PeriodReportData& data) const {
    ss << std::format("## {0} {1} {2} ({3} {4} {5})\n\n",
        config_->get_report_title_prefix(),
        data.days_to_query,
        config_->get_report_title_days(),
        data.start_date,
        config_->get_report_title_date_separator(),
        data.end_date
    );

    if (data.actual_days > 0) {
        ss << std::format("- **{0}**: {1}\n", config_->get_total_time_label(), time_format_duration(data.total_duration, data.actual_days));
        ss << std::format("- **{0}**: {1}\n", config_->get_actual_days_label(), data.actual_days);
    }
}

void PeriodMd::_display_project_breakdown(std::stringstream& ss, const PeriodReportData& data) const {
    ss << MarkdownUtils::format_project_tree(data.project_tree, data.total_duration, data.actual_days);
}