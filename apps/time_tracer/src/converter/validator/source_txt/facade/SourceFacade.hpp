// converter/validator/source_txt/facade/SourceFacade.hpp
#ifndef SOURCE_FACADE_HPP
#define SOURCE_FACADE_HPP

#include "converter/validator/common/ValidatorUtils.hpp"
#include "converter/convert/config/ConverterConfig.hpp"
#include <string>
#include <set>
#include <memory>

class SourceFacade {
public:
    explicit SourceFacade(const ConverterConfig& config);
    ~SourceFacade();

    // [核心修改] 接收文件名(仅用于日志)和内容字符串
    bool validate(const std::string& filename, const std::string& content, std::set<Error>& errors);

private:
    struct PImpl;
    std::unique_ptr<PImpl> pimpl_;
};

#endif // SOURCE_FACADE_HPP