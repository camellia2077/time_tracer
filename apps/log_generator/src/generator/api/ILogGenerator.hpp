// generator/api/ILogGenerator.hpp
#ifndef I_LOG_GENERATOR_HPP
#define I_LOG_GENERATOR_HPP

#include <string>

class ILogGenerator {
public:
    virtual ~ILogGenerator() = default;
    // [修改] 改为 void 返回，增加 buffer 引用参数
    virtual void generate_for_month(int year, int month, int days_in_month, std::string& buffer) = 0;
};

#endif // I_LOG_GENERATOR_HPP