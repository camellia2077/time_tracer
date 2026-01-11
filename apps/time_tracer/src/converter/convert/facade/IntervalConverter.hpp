// converter/convert/facade/IntervalConverter.hpp
#ifndef INTERVAL_CONVERTER_HPP
#define INTERVAL_CONVERTER_HPP

#include "converter/convert/config/ConverterConfig.hpp"
#include "converter/convert/model/InputData.hpp"
#include <string>
#include <istream>
#include <vector> 

class IntervalConverter {
public:
    // [核心修改] 依赖注入配置对象，而非路径
    explicit IntervalConverter(const ConverterConfig& config);

    std::vector<InputData> executeConversion(std::istream& combined_input_stream);

private:
    const ConverterConfig& config_; // 持有引用或拷贝，这里选择引用假定调用者生命周期更长，或者拷贝更安全。为了安全，这里改为拷贝或引用取决于 LogProcessor。
    // 鉴于 Config 可能较大，通常引用。但此处 LogProcessor 持有 Config，所以引用是安全的。
};

#endif // INTERVAL_CONVERTER_HPP