// reprocessing/validator/internal/JsonValidator.hpp
#ifndef JSON_VALIDATOR_HPP
#define JSON_VALIDATOR_HPP

#include "reprocessing/validator/common/ValidatorUtils.hpp"
#include <string>
#include <set>
#include <nlohmann/json.hpp>

// 这个模块专门负责验证经过转换后生成的 JSON 输出文件 的数据结构和内容

/**
 * @class JsonValidator
 * @brief 用于验证JSON输出文件格式和内容正确性的类。
 *
 * 这个类提供了一套完整的验证流程，用于检查一个JSON文件是否符合预定义的结构和逻辑规则。
 * 它可以执行以下检查：
 * 1.  **文件可访问性**：确保文件可以被成功打开和读取。
 * 2.  **基本JSON结构**：验证文件的根元素是否为一个数组。
 * 3.  **日期连续性**：(可选) 检查一个月中的所有日期是否都存在，没有缺失。
 * 4.  **时间连续性**：检查一天内各个活动(Activity)的起止时间是否完美衔接，没有间断或重叠。
 * 5.  **高级业务逻辑**：
 * -   如果某天的"Sleep"标记为true，那么该天的最后一个活动必须是"sleep"。
 */
class JsonValidator {
public:
    /**
     * @brief 构造函数。
     * @param enable_day_count_check (可选) 一个布尔值，用于决定是否启用对一个月内日期完整性的检查。
     * 默认为false，即不检查。
     */
    explicit JsonValidator(bool enable_day_count_check = false);

    /**
     * @brief 对指定的JSON文件执行所有已启用的验证规则。
     * @param file_path 要验证的JSON文件的路径。
     * @param errors 一个 `std::set<Error>` 的引用，用于收集验证过程中发现的所有错误。
     * 函数执行前会清空该集合。
     * @return 如果文件通过所有验证，则返回true；否则返回false。
     */
    bool validate(const std::string& file_path, std::set<Error>& errors);

private:
    /// @brief 一个布尔标志，用于存储是否启用日期计数检查的配置。
    bool check_day_count_enabled_;

    // --- 内部验证的辅助函数 ---

    /**
     * @brief 验证一个月内日期的连续性和完整性。
     * @param days_array 包含所有天(day)对象的JSON数组。
     * @param errors 用于存储错误的集合。
     * @note  该函数会检查从该月第一天到最后一天的所有日期是否存在于文件中，并将缺失的日期作为错误报告。
     */
    void validateDateContinuity(const nlohmann::json& days_array, std::set<Error>& errors);
    
    /**
     * @brief 验证单日内所有活动的时间是否连续。
     * @param day_object 代表一天的JSON对象。
     * @param errors 用于存储错误的集合。
     * @note  它会检查前一个活动的 `endTime` 是否与后一个活动的 `startTime` 完全匹配。
     */
    void validateTimeContinuity(const nlohmann::json& day_object, std::set<Error>& errors);
    
    /**
     * @brief 验证与业务逻辑相关的高级规则。
     * @param day_object 代表一天的JSON对象。
     * @param errors 用于存储错误的集合。
     * @note  例如，检查当 `Headers.Sleep` 为true时，最后一个活动是否为 'sleep'。
     */
    void validateHighLevelRules(const nlohmann::json& day_object, std::set<Error>& errors);
};

#endif // JSON_VALIDATOR_HPP