// queries/daily/formatters/tex/DayTex.hpp
#ifndef DAILY_REPORT_TEX_FORMATTER_HPP
#define DAILY_REPORT_TEX_FORMATTER_HPP

#include "queries/shared/interfaces/IReportFormatter.hpp"
#include "queries/shared/data/DailyReportData.hpp"
#include "queries/daily/formatters/tex/DayTexConfig.hpp" // [修改] 引入新的配置类头文件
#include <memory>
#include <sstream>

class DayTex : public IReportFormatter<DailyReportData> {
public:
    /**
     * @brief [修改] 构造函数现在接收一个指向配置对象的共享指针。
     */
    explicit DayTex(std::shared_ptr<DayTexConfig> config);

    std::string format_report(const DailyReportData& data) const override;

private:
    // 私有方法保持不变
    void _display_header(std::stringstream& ss, const DailyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const DailyReportData& data) const;
    void _display_statistics(std::stringstream& ss, const DailyReportData& data) const;
    void _display_detailed_activities(std::stringstream& ss, const DailyReportData& data) const;
    
    /**
     * @brief [新增] 用于存储配置对象的成员变量。
     */
    std::shared_ptr<DayTexConfig> config_;
};

#endif // DAILY_REPORT_TEX_FORMATTER_HPP