// reprocessing/validator/source_txt/pipelines/StructuralValidator.hpp
#ifndef STRUCTURAL_VALIDATOR_HPP
#define STRUCTURAL_VALIDATOR_HPP

#include "reprocessing/validator/common/ValidatorUtils.hpp"
#include <string>
#include <set>
// 负责跟踪验证进度并强制执行文件结构规则
// 负责管理验证状态并检查文件结构/逻辑顺序
class StructuralValidator {
public:
    StructuralValidator() = default;

    void process_year_line(int line_number, const std::string& line, std::set<Error>& errors);
    void process_date_line(int line_number, const std::string& line, std::set<Error>& errors);
    void process_remark_line(int line_number, const std::string& line, std::set<Error>& errors);
    void process_event_line(int line_number, const std::string& line, std::set<Error>& errors);
    void process_unrecognized_line(int line_number, const std::string& line, std::set<Error>& errors);
    
    bool has_seen_year() const;

private:
    bool has_seen_year_ = false;
    bool has_seen_date_in_block_ = false;
    bool has_seen_event_in_day_ = false;
};

#endif // STRUCTURAL_VALIDATOR_HPP