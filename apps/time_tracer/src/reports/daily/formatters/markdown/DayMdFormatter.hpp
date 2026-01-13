// reports/daily/formatters/markdown/DayMdFormatter.hpp
#ifndef DAY_MD_FORMATTER_HPP
#define DAY_MD_FORMATTER_HPP

#include "reports/shared/formatters/templates/BaseMdFormatter.hpp"
#include "reports/shared/model/DailyReportData.hpp"
#include "reports/daily/formatters/markdown/DayMdConfig.hpp"

class DayMdFormatter : public BaseMdFormatter<DailyReportData, DayMdConfig> {
public:
    explicit DayMdFormatter(std::shared_ptr<DayMdConfig> config);

protected:
    // --- 实现基类钩子 ---
    bool is_empty_data(const DailyReportData& data) const override;
    int get_avg_days(const DailyReportData& data) const override;
    
    void format_header_content(std::stringstream& ss, const DailyReportData& data) const override;
    void format_extra_content(std::stringstream& ss, const DailyReportData& data) const override;

    // 适配接口：DayConfig 使用 get_no_records() 而非 get_no_records_message()
    std::string get_no_records_msg() const override;

private:
    //原本的私有辅助函数现在变成了钩子函数的具体实现，或者被内联
    void _display_detailed_activities(std::stringstream& ss, const DailyReportData& data) const;
};

#endif // DAY_MD_FORMATTER_HPP