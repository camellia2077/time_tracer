// domain/api/i_log_generator.hpp
#ifndef DOMAIN_API_I_LOG_GENERATOR_H_
#define DOMAIN_API_I_LOG_GENERATOR_H_

#include <string>

class ILogGenerator {
 public:
  virtual ~ILogGenerator() = default;
  virtual void generate_for_month(int year, int month, int days_in_month,
                                  std::string& buffer) = 0;
};

#endif  // DOMAIN_API_I_LOG_GENERATOR_H_
