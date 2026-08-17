// infra/insights/shared/factories/generic_formatter_factory.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_SHARED_FACTORIES_GENERIC_FORMATTER_FACTORY_H_
#define INFRASTRUCTURE_INSIGHTS_SHARED_FACTORIES_GENERIC_FORMATTER_FACTORY_H_

#include <functional>
#include <map>
#include <memory>
#include <stdexcept>

#include "domain/insights/types/insights_types.hpp"
#include "infra/config/models/insights_catalog.hpp"
#include "infra/insights/shared/interfaces/i_insights_formatter.hpp"

template <typename InsightsDataType>
class GenericFormatterFactory {
 public:
  using Creator =
      std::function<std::unique_ptr<IInsightsFormatter<InsightsDataType>>(
          const InsightsCatalog&)>;

  [[nodiscard]] static auto Create(InsightsFormat format,
                                   const InsightsCatalog& catalog)
      -> std::unique_ptr<IInsightsFormatter<InsightsDataType>> {
    auto& creators = GetCreators();
    auto iter = creators.find(format);

    if (iter == creators.end()) {
      throw std::invalid_argument(
          "Unsupported insights format or formatter not registered for this "
          "data "
          "type.");
    }

    return iter->second(catalog);
  }

  static void RegisterCreator(InsightsFormat format, Creator creator) {
    GetCreators()[format] = std::move(creator);
  }

  static void UnregisterCreator(InsightsFormat format) {
    GetCreators().erase(format);
  }

  static auto GetCreators() -> std::map<InsightsFormat, Creator>& {
    static std::map<InsightsFormat, Creator> creators;
    return creators;
  }
};

#endif  // INFRASTRUCTURE_INSIGHTS_SHARED_FACTORIES_GENERIC_FORMATTER_FACTORY_H_
