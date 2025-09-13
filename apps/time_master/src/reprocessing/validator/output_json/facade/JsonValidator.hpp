// reprocessing/validator/internal/JsonValidator.hpp
#ifndef JSON_VALIDATOR_HPP
#define JSON_VALIDATOR_HPP

#include "reprocessing/validator/common/ValidatorUtils.hpp"
#include <string>
#include <set>
#include <nlohmann/json.hpp>

#include "reprocessing/validator/output_json/pipelines/JsonValidatorDate.hpp"
#include "reprocessing/validator/output_json/pipelines/JsonValidatorTime.hpp"
#include "reprocessing/validator/output_json/pipelines/JsonValidatorRules.hpp"
#include "reprocessing/validator/output_json/pipelines/JsonValidatorActivities.hpp"

// 这个模块专门负责验证经过转换后生成的 JSON 输出文件 的数据结构和内容
/**
 * @class JsonValidator
 * @brief 用于验证JSON输出文件格式和内容正确性的类，作为多个独立验证模块的外观（Facade）。
 *
 * 这个类提供了一套完整的验证流程，用于检查一个JSON文件是否符合预定义的结构和逻辑规则。
 * 它的公共接口保持不变，但内部实现已拆分为独立的函数。
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
};

#endif // JSON_VALIDATOR_HPP