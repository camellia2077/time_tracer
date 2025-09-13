#ifndef SOURCE_FILE_VALIDATOR_FACADE_HPP
#define SOURCE_FILE_VALIDATOR_FACADE_HPP

#include "reprocessing/validator/common/ValidatorUtils.hpp"
#include <string>
#include <set>
#include <memory>

// 前向声明内部组件，避免在头文件中暴露实现细节
class SourceValidatorConfig;
class LineProcessor;
class StructuralValidator;

/**
 * @class SourceFileValidator
 * @brief (Facade) 协调多个子验证器来完成对源文件的全面验证。
 *
 * 此类作为验证子系统的统一入口，封装了内部复杂的交互逻辑。
 */
class SourceFileValidator {
public:
    explicit SourceFileValidator(const std::string& config_filename);
    ~SourceFileValidator(); // 需要析构函数来处理 pImpl

    bool validate(const std::string& file_path, std::set<Error>& errors);

private:
    // 使用 PImpl (Pointer to Implementation) 模式隐藏内部成员
    struct PImpl;
    std::unique_ptr<PImpl> pimpl_;
};

#endif // SOURCE_FILE_VALIDATOR_FACADE_HPP