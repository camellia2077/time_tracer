// infra/insights/services/batch_export_helpers.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_SERVICES_BATCH_EXPORT_HELPERS_H_
#define INFRASTRUCTURE_INSIGHTS_SERVICES_BATCH_EXPORT_HELPERS_H_

#include "infra/sqlite_fwd.hpp"

#include "domain/insights/interfaces/i_project_info_provider.hpp"
#include "infra/insights/data/cache/project_name_cache.hpp"
#include "infra/insights/data/utils/project_tree_builder.hpp"

namespace insights::services {

inline auto CreateProjectNameCache(sqlite3* sqlite_db) -> ProjectNameCache {
  ProjectNameCache cache;
  cache.EnsureLoaded(sqlite_db);
  return cache;
}

template <typename InsightsDataT>
inline void EnsureProjectTree(InsightsDataT& data,
                              const IProjectInfoProvider& provider) {
  if (data.total_duration <= 0 || !data.project_tree.empty()) {
    return;
  }
  BuildProjectTreeFromIds(data.project_tree, data.project_stats, provider);
}

template <typename MapT, typename FormatterT, typename InserterT>
inline void FormatInsightsMap(MapT& data_map, FormatterT& formatter,
                            const IProjectInfoProvider& provider,
                            InserterT insert) {
  for (auto& [key, data] : data_map) {
    if (data.total_duration <= 0) {
      continue;
    }
    EnsureProjectTree(data, provider);
    insert(key, formatter->FormatInsights(data));
  }
}

}  // namespace insights::services

#endif  // INFRASTRUCTURE_INSIGHTS_SERVICES_BATCH_EXPORT_HELPERS_H_
