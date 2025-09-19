// reprocessing/validator/source_txt/facade/SourceFileValidator.hpp
#ifndef SOURCE_FILE_VALIDATOR_FACADE_HPP
#define SOURCE_FILE_VALIDATOR_FACADE_HPP

#include "reprocessing/validator/common/ValidatorUtils.hpp"
#include "reprocessing/converter/config/ConverterConfig.hpp" // [新增]
#include <string>
#include <set>
#include <memory>

// 前向声明内部组件
class LineProcessor;
class StructuralValidator;

/**
 * @class SourceFileValidator
 * @brief (Facade) 协调多个子验证器来完成对源文件的全面验证。
 */
class SourceFileValidator {
public:
    // [修改] 构造函数现在接收 ConverterConfig
    explicit SourceFileValidator(const ConverterConfig& config);
    ~SourceFileValidator();

    bool validate(const std::string& file_path, std::set<Error>& errors);

private:
    struct PImpl;
    std::unique_ptr<PImpl> pimpl_;
};

#endif // SOURCE_FILE_VALIDATOR_FACADE_HPP