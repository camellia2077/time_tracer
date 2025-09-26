// reprocessing/validator/source_txt/facade/SourceFacade.hpp
#ifndef SOURCE_FACADE_HPP
#define SOURCE_FACADE_HPP

#include "reprocessing/validator/common/ValidatorUtils.hpp"
#include "reprocessing/converter/config/ConverterConfig.hpp" // [新增]
#include <string>
#include <set>
#include <memory>

// 前向声明内部组件
class LineRules;
class StructureRules;

/**
 * @class SourceFacade
 * @brief (Facade) 协调多个子验证器来完成对源文件的全面验证。
 */
class SourceFacade {
public:
    // [修改] 构造函数现在接收 ConverterConfig
    explicit SourceFacade(const ConverterConfig& config);
    ~SourceFacade();

    bool validate(const std::string& file_path, std::set<Error>& errors);

private:
    struct PImpl;
    std::unique_ptr<PImpl> pimpl_;
};

#endif // SOURCE_FACADE_HPP