// config/validator/reports/facade/query_facade.hpp
#ifndef CONFIG_VALIDATOR_REPORTS_FACADE_QUERY_FACADE_H_
#define CONFIG_VALIDATOR_REPORTS_FACADE_QUERY_FACADE_H_

#include <toml++/toml.h>

#include <string>
#include <vector>

class QueryFacade {
 public:
  static bool validate(
      const std::vector<std::pair<std::string, toml::table>>& query_configs);
};

#endif  // CONFIG_VALIDATOR_REPORTS_FACADE_QUERY_FACADE_H_