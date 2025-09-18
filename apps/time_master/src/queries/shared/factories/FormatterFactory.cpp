// queries/shared/factories/FormatterFactory.cpp
#include "FormatterFactory.hpp"

// Include all necessary data structures, configs, and formatters
#include "queries/shared/data/DailyReportData.hpp"
#include "queries/daily/formatters/md/DayMd.hpp"
#include "queries/daily/formatters/md/DayMdConfig.hpp"
#include "queries/daily/formatters/tex/DayTex.hpp"
#include "queries/daily/formatters/tex/DayTexConfig.hpp"
#include "queries/daily/formatters/typ/DayTyp.hpp"
#include "queries/daily/formatters/typ/DayTypConfig.hpp"

#include "queries/shared/data/MonthlyReportData.hpp"
#include "queries/monthly/formatters/md/MonthMd.hpp"
#include "queries/monthly/formatters/md/MonthMdConfig.hpp"
#include "queries/monthly/formatters/tex/MonthTex.hpp"
#include "queries/monthly/formatters/tex/MonthTexConfig.hpp"
#include "queries/monthly/formatters/typ/MonthTyp.hpp"
#include "queries/monthly/formatters/typ/MonthTypConfig.hpp"

#include "queries/shared/data/PeriodReportData.hpp"
#include "queries/period/formatters/md/PeriodMd.hpp"
#include "queries/period/formatters/md/PeriodMdConfig.hpp"
#include "queries/period/formatters/tex/PeriodTex.hpp"
#include "queries/period/formatters/tex/PeriodTexConfig.hpp"
#include "queries/period/formatters/typ/PeriodTyp.hpp"
#include "queries/period/formatters/typ/PeriodTypConfig.hpp"

#include <stdexcept>

std::unique_ptr<IReportFormatter<DailyReportData>> 
FormatterFactory::create_day_formatter(ReportFormat format, const AppConfig& config) {
    switch (format) {
        case ReportFormat::Typ: {
            auto typ_config = std::make_shared<DayTypConfig>(config.day_typ_config_path);
            return std::make_unique<DayTyp>(typ_config);
        }
        case ReportFormat::Markdown: {
            auto md_config = std::make_shared<DayMdConfig>(config.day_md_config_path);
            return std::make_unique<DayMd>(md_config);
        }
        case ReportFormat::LaTeX: {
            auto tex_config = std::make_shared<DayTexConfig>(config.day_tex_config_path);
            return std::make_unique<DayTex>(tex_config);
        }
        default:
            throw std::invalid_argument("Unsupported report format for daily reports.");
    }
}

std::unique_ptr<IReportFormatter<MonthlyReportData>> 
FormatterFactory::create_month_formatter(ReportFormat format, const AppConfig& config) {
    switch (format) {
        case ReportFormat::Typ: {
            auto typ_config = std::make_shared<MonthTypConfig>(config.month_typ_config_path);
            return std::make_unique<MonthTyp>(typ_config);
        }
        case ReportFormat::Markdown: {
            auto md_config = std::make_shared<MonthMdConfig>(config.month_md_config_path);
            return std::make_unique<MonthMd>(md_config);
        }
        case ReportFormat::LaTeX: {
            auto tex_config = std::make_shared<MonthTexConfig>(config.month_tex_config_path);
            return std::make_unique<MonthTex>(tex_config);
        }
        default:
            throw std::invalid_argument("Unsupported report format for monthly reports.");
    }
}

std::unique_ptr<IReportFormatter<PeriodReportData>> 
FormatterFactory::create_period_formatter(ReportFormat format, const AppConfig& config) {
    switch (format) {
        case ReportFormat::Typ: {
            auto typ_config = std::make_shared<PeriodTypConfig>(config.period_typ_config_path);
            return std::make_unique<PeriodTyp>(typ_config);
        }
        case ReportFormat::Markdown: {
            auto md_config = std::make_shared<PeriodMdConfig>(config.period_md_config_path);
            return std::make_unique<PeriodMd>(md_config);
        }
        case ReportFormat::LaTeX: {
            auto tex_config = std::make_shared<PeriodTexConfig>(config.period_tex_config_path);
            return std::make_unique<PeriodTex>(tex_config);
        }
        default:
            throw std::invalid_argument("Unsupported report format for period reports.");
    }
}