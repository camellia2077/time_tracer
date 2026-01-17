// validator/txt/facade/TextValidator.hpp
#ifndef VALIDATOR_TXT_FACADE_TEXT_VALIDATOR_HPP_
#define VALIDATOR_TXT_FACADE_TEXT_VALIDATOR_HPP_

#include "validator/common/ValidatorUtils.hpp"
#include "converter/config/ConverterConfig.hpp"

#include <string>
#include <set>
#include <memory>

namespace validator {
namespace txt {

class TextValidator {
public:
    explicit TextValidator(const ConverterConfig& config);
    ~TextValidator();

    bool validate(const std::string& filename, const std::string& content, std::set<Error>& errors);

private:
    struct PImpl;
    std::unique_ptr<PImpl> pimpl_;
};

} // namespace txt
} // namespace validator

#endif // VALIDATOR_TXT_FACADE_TEXT_VALIDATOR_HPP_