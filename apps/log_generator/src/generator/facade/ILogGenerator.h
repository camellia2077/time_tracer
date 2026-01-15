#ifndef GENERATOR_FACADE_ILOGGENERATOR_H
#define GENERATOR_FACADE_ILOGGENERATOR_H

#include <string>

class ILogGenerator {
public:
    virtual ~ILogGenerator() = default;
    virtual std::string generate_for_month(int year, int month, int days_in_month) = 0;
};

#endif // GENERATOR_FACADE_ILOGGENERATOR_H