// queries/shared/factories/FormatterFactory.hpp
#ifndef FORMATTER_FACTORY_HPP
#define FORMATTER_FACTORY_HPP

#include <memory>
#include "queries/shared/ReportFormat.hpp"
#include "queries/shared/Interface/IReportFormatter.hpp"
#include "common/AppConfig.hpp"

// Forward declare data types to avoid including heavy headers
struct DailyReportData;
struct MonthlyReportData;
struct PeriodReportData;

class FormatterFactory {
public:
    /**
     * @brief Creates a formatter for daily reports.
     * @param format The desired report format.
     * @param config The application configuration containing config file paths.
     * @return A unique pointer to the created formatter.
     */
    static std::unique_ptr<IReportFormatter<DailyReportData>> 
    create_day_formatter(ReportFormat format, const AppConfig& config);

    /**
     * @brief Creates a formatter for monthly reports.
     * @param format The desired report format.
     * @param config The application configuration.
     * @return A unique pointer to the created formatter.
     */
    static std::unique_ptr<IReportFormatter<MonthlyReportData>> 
    create_month_formatter(ReportFormat format, const AppConfig& config);

    /**
     * @brief Creates a formatter for period reports.
     * @param format The desired report format.
     * @param config The application configuration.
     * @return A unique pointer to the created formatter.
     */
    static std::unique_ptr<IReportFormatter<PeriodReportData>> 
    create_period_formatter(ReportFormat format, const AppConfig& config);
};

#endif // FORMATTER_FACTORY_HPP