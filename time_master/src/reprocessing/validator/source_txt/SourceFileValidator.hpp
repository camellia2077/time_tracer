// reprocessing/validator/internal/SourceFileValidator.hpp
#ifndef SOURCE_FILE_VALIDATOR_HPP
#define SOURCE_FILE_VALIDATOR_HPP

#include "reprocessing/validator/common/ValidatorUtils.hpp"

#include <string>
#include <set>

#include <unordered_set>
#include <nlohmann/json.hpp>

/**
 * @class SourceFileValidator
 * @brief 用于验证原始文本源文件格式的类。
 *
 * 此类旨在确保输入的源文件遵循一套严格的格式规则，以便后续程序可以正确解析。
 * 它主要执行以下检查：
 * 1.  **文件结构**：
 * -   文件的第一个非空行必须是年份标题 (例如, 'y2025')。
 * -   年份之后必须紧跟日期行 (例如, '0101')。
 * -   日期行之后可以有备注行或事件行。
 * 2.  **行格式**：
 * -   识别并验证日期行 (MMDD)。
 * -   识别并验证备注行 (以特定前缀开头, 例如 'r ')。
 * -   识别并验证事件行 (HHMMdescription)。
 * 3.  **内容逻辑**：
 * -   备注行不能出现在当天的任何事件行之后。
 * -   当天的第一个事件必须是预定义的“唤醒活动”(例如, '起床')。
 * -   所有事件的描述都必须是配置文件中定义的有效关键字。
 */
class SourceFileValidator {
public:
    /**
     * @brief 构造函数，需要一个配置文件路径来加载验证规则。
     * @param config_filename 包含备注前缀、有效事件关键字和唤醒关键字的JSON配置文件路径。
     */
    explicit SourceFileValidator(const std::string& config_filename);
    
    /**
     * @brief 对指定的源文件执行验证。
     * @param file_path 要验证的源文件的路径。
     * @param errors 一个 `std::set<Error>` 的引用，用于收集所有发现的格式错误。
     * @return 如果文件格式完全正确，则返回true；否则返回false。
     */
    bool validate(const std::string& file_path, std::set<Error>& errors);

private:
    // --- 成员变量 ---
    std::string remark_prefix_;                           ///< 从配置中加载的备注行前缀。
    std::unordered_set<std::string> valid_event_keywords_; ///< 所有有效的事件关键字集合。
    std::unordered_set<std::string> wake_keywords_;        ///< 当天第一个事件必须匹配的唤醒关键字集合。

    // --- 私有辅助函数 ---

    /**
     * @brief 从JSON文件中加载配置信息（备注前缀、事件关键字等）。
     * @param config_filename 配置文件路径。
     */
    void loadConfiguration(const std::string& config_filename);
    
    /**
     * @brief 检查给定行是否为年份行 (例如, 'y2025')。
     * @param line 要检查的字符串行。
     * @return 如果是年份行，返回true。
     */
    bool isYearLine(const std::string& line);
    
    /**
     * @brief 检查给定行是否为日期行 (例如, '0101')。
     * @param line 要检查的字符串行。
     * @return 如果是日期行，返回true。
     */
    bool isDateLine(const std::string& line);
    
    /**
     * @brief 检查给定行是否为备注行 (以 `remark_prefix_` 开头)。
     * @param line 要检查的字符串行。
     * @return 如果是备注行，返回true。
     */
    bool isRemarkLine(const std::string& line); 
    
    /**
     * @brief 解析并验证一个事件行。
     * @param line 要解析的字符串行。
     * @param errors 用于存储错误的集合。
     * @param line_number 当前的行号，用于生成错误信息。
     * @param is_first_event 一个布尔值，指示这是否是当天的第一个事件。
     * @return 如果行格式符合事件行的基本格式 (HHMMdescription)，则返回true，即使事件内容无效。
     * 如果行格式完全不匹配，则返回false。
     */
    bool parseAndValidateEventLine(const std::string& line, std::set<Error>& errors, int line_number, bool is_first_event);
};

#endif // SOURCE_FILE_VALIDATOR_HPP