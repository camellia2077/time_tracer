// domain/api/i_log_generator.hpp
#ifndef DOMAIN_API_I_LOG_GENERATOR_H_
#define DOMAIN_API_I_LOG_GENERATOR_H_

#include <string>

struct MonthContext {
  int year;
  int month;
  int days_in_month;
};

class ILogGenerator {
 public:
  virtual ~ILogGenerator() = default;
  virtual void generate_for_month(const MonthContext& month_context,
                                  std::string& buffer) = 0;
};

#endif  // DOMAIN_API_I_LOG_GENERATOR_H_
