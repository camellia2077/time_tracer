#ifndef DAILY_REPORT_FORMATTER_FACTORY_H
#define DAILY_REPORT_FORMATTER_FACTORY_H

#include "IReportFormatter.h"
#include "report_generators/_shared/ReportFormat.h"
#include <memory>

/**
 * @class DailyReportFormatterFactory
 * @brief 负责创建具体日报格式化器实例的工厂。
 */
class DailyReportFormatterFactory {
public:
    /**
     * @brief 根据指定的格式创建一个格式化器实例。
     * @param format 期望的报告格式。
     * @return 一个指向 IReportFormatter 接口的智能指针。
     * @throws std::invalid_argument 如果请求了不支持的格式。
     */
    static std::unique_ptr<IReportFormatter> create_formatter(ReportFormat format);
};

#endif // DAILY_REPORT_FORMATTER_FACTORY_H