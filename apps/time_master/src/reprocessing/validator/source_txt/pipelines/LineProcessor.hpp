// reprocessing/validator/source_txt/pipelines/LineProcessor.hpp
#ifndef LINE_PROCESSOR_HPP
#define LINE_PROCESSOR_HPP

#include "SourceValidatorConfig.hpp"
#include "reprocessing/validator/common/ValidatorUtils.hpp"
#include <string>
#include <set>
// 这个类负责分析和验证单行文本的内容。
// 负责处理单行文本的验证逻辑
class LineProcessor {
public:
    explicit LineProcessor(const SourceValidatorConfig& config);
    
    bool is_year(const std::string& line) const;
    bool is_date(const std::string& line) const;
    bool is_remark(const std::string& line) const;
    
    // 验证事件行的格式和内容，如果格式正确则返回true
    bool is_valid_event_line(const std::string& line, int line_number, std::set<Error>& errors) const;

private:
    const SourceValidatorConfig& config_;
};

#endif // LINE_PROCESSOR_HPP