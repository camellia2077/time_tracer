// infrastructure/config/validator/reports/strategies/report_query_strategies.hpp
#ifndef INFRASTRUCTURE_CONFIG_VALIDATOR_REPORTS_STRATEGIES_REPORT_QUERY_STRATEGIES_H_
#define INFRASTRUCTURE_CONFIG_VALIDATOR_REPORTS_STRATEGIES_REPORT_QUERY_STRATEGIES_H_

#include <toml++/toml.h>

#include <memory>
#include <string>

class IQueryStrategy {
 public:
  virtual ~IQueryStrategy() = default;
  [[nodiscard]] virtual auto Validate(const toml::table& query_config,
                                      const std::string& file_name) const
      -> bool = 0;
};

class BaseStrategy : public IQueryStrategy {
 public:
  [[nodiscard]] auto Validate(const toml::table& query_config,
                              const std::string& file_name) const -> bool final;
  ~BaseStrategy() override = default;

 protected:
  [[nodiscard]] virtual auto ValidateSpecificKeys(
      const toml::table& query_config, const std::string& file_name) const
      -> bool = 0;

 private:
  static auto ValidateCommonRules(const toml::table& query_config,
                                  const std::string& file_name) -> bool;
  static auto ValidateStatisticsItems(const toml::table& query_config,
                                      const std::string& file_name) -> bool;
  static auto IsValidHexColor(const std::string& color_string) -> bool;
};

class DailyMd : public BaseStrategy {
 protected:
  [[nodiscard]] auto ValidateSpecificKeys(const toml::table& query_config,
                                          const std::string& file_name) const
      -> bool override;
};

class DailyTex : public BaseStrategy {
 protected:
  [[nodiscard]] auto ValidateSpecificKeys(const toml::table& query_config,
                                          const std::string& file_name) const
      -> bool override;
};

class DailyTyp : public BaseStrategy {
 protected:
  [[nodiscard]] auto ValidateSpecificKeys(const toml::table& query_config,
                                          const std::string& file_name) const
      -> bool override;
};

class Monthly : public BaseStrategy {
 protected:
  [[nodiscard]] auto ValidateSpecificKeys(const toml::table& query_config,
                                          const std::string& file_name) const
      -> bool override;
};

class Weekly : public BaseStrategy {
 protected:
  [[nodiscard]] auto ValidateSpecificKeys(const toml::table& query_config,
                                          const std::string& file_name) const
      -> bool override;
};

class Yearly : public BaseStrategy {
 protected:
  [[nodiscard]] auto ValidateSpecificKeys(const toml::table& query_config,
                                          const std::string& file_name) const
      -> bool override;
};

class StrategyFactory {
 public:
  static auto CreateStrategy(const std::string& file_name)
      -> std::unique_ptr<IQueryStrategy>;
};

#endif  // INFRASTRUCTURE_CONFIG_VALIDATOR_REPORTS_STRATEGIES_REPORT_QUERY_STRATEGIES_H_
