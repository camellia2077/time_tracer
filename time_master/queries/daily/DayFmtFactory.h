// queries/report_generators/daily/formatter/DayFmtFactory.h
#ifndef DAILY_FMT_FACTORY_H
#define DAILY_FMT_FACTORY_H

#include "queries/shared/Interface/IReportFormatter.h" // Replaced IDayFmt.h
#include "queries/shared/DailyReportData.h"   // Added for the template type
#include "queries/shared/ReportFormat.h"
#include <memory>

/**
 * @class DayFmtFactory
 * @brief 负责创建具体日报格式化器实例的工厂。
 */
class DayFmtFactory {
public:
    /**
     * @brief 根据指定的格式创建一个格式化器实例。
     * @param format 期望的报告格式。
     * @return 一个指向 IReportFormatter<DailyReportData> 接口的智能指针。
     * @throws std::invalid_argument 如果请求了不支持的格式。
     */
    static std::unique_ptr<IReportFormatter<DailyReportData>> create_formatter(ReportFormat format);
};

#endif // DAILY_FMT_FACTORY_H