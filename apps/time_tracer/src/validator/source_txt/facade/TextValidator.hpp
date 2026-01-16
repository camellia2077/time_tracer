// validator/source_txt/facade/TextValidator.hpp
#ifndef TEXT_VALIDATOR_HPP
#define TEXT_VALIDATOR_HPP

#include "validator/common/ValidatorUtils.hpp"
#include "converter/config/ConverterConfig.hpp"

#include <string>
#include <set>
#include <memory>

class TextValidator {
public:
    explicit TextValidator(const ConverterConfig& config);
    ~TextValidator();

    bool validate(const std::string& filename, const std::string& content, std::set<Error>& errors);

private:
    struct PImpl;
    std::unique_ptr<PImpl> pimpl_;
};

#endif // TEXT_VALIDATOR_HPP