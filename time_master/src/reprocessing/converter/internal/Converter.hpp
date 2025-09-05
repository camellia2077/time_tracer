// reprocessing/converter/internal/Converter.hpp
#ifndef INTERVAL_CONVERTER_HPP
#define INTERVAL_CONVERTER_HPP

#include "reprocessing/converter/model/InputData.hpp"
#include "ConverterConfig.hpp"
#include <string>
#include <unordered_set>

class Converter {
public:
    explicit Converter(const ConverterConfig& config);

    void transform(InputData& day);

private:
    const ConverterConfig& config_;
    // [优化] 将 unordered_set 作为成员变量，避免在 transform 中重复创建
    const std::unordered_set<std::string> wake_keywords_;
};

#endif // INTERVAL_CONVERTER_HPP