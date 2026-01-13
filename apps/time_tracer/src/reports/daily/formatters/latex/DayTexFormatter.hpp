// reports/daily/formatters/latex/DayTexFormatter.hpp
#ifndef DAY_TEX_FORMATTER_HPP
#define DAY_TEX_FORMATTER_HPP

#include "reports/shared/formatters/templates/BaseTexFormatter.hpp"
#include "reports/shared/model/DailyReportData.hpp"
#include "reports/daily/formatters/latex/DayTexConfig.hpp"

class DayTexFormatter : public BaseTexFormatter<DailyReportData, DayTexConfig> {
public:
    explicit DayTexFormatter(std::shared_ptr<DayTexConfig> config);

protected:
    // --- 实现基类钩子 ---
    bool is_empty_data(const DailyReportData& data) const override;
    int get_avg_days(const DailyReportData& data) const override;
    
    void format_header_content(std::stringstream& ss, const DailyReportData& data) const override;
    void format_extra_content(std::stringstream& ss, const DailyReportData& data) const override;
    
    std::map<std::string, std::string> get_keyword_colors() const override;

    // [新增] 实现基类定义的纯虚函数
    std::string get_no_records_msg() const override;
};

#endif // DAY_TEX_FORMATTER_HPP