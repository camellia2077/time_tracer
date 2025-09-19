// reprocessing/validator/source_txt/pipelines/LineProcessor.hpp
#ifndef LINE_PROCESSOR_HPP
#define LINE_PROCESSOR_HPP

#include "reprocessing/converter/config/ConverterConfig.hpp" // [修改] 引用 ConverterConfig
#include "reprocessing/validator/common/ValidatorUtils.hpp"
#include <string>
#include <set>
#include <unordered_set> // [新增]

// 这个类负责分析和验证单行文本的内容。
// 负责处理单行文本的验证逻辑
class LineProcessor {
public:
    // [修改] 构造函数现在接收 ConverterConfig
    explicit LineProcessor(const ConverterConfig& config);
    
    bool is_year(const std::string& line) const;
    bool is_date(const std::string& line) const;
    bool is_remark(const std::string& line) const;
    
    // 验证事件行的格式和内容，如果格式正确则返回true
    bool is_valid_event_line(const std::string& line, int line_number, std::set<Error>& errors) const;

private:
    // [修改] 成员变更为 ConverterConfig 引用和从其派生的关键字集合
    const ConverterConfig& config_;
    std::unordered_set<std::string> valid_event_keywords_;
    std::unordered_set<std::string> wake_keywords_;
};

#endif // LINE_PROCESSOR_HPP