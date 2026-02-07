// infrastructure/reports/services/batch_export_helpers.hpp
#ifndef REPORTS_SERVICES_BATCH_EXPORT_HELPERS_H_
#define REPORTS_SERVICES_BATCH_EXPORT_HELPERS_H_

#include <sqlite3.h>

#include "domain/reports/interfaces/i_project_info_provider.hpp"
#include "infrastructure/reports/data/cache/project_name_cache.hpp"
#include "infrastructure/reports/data/utils/project_tree_builder.hpp"

namespace reports::services {

inline auto EnsureProjectNameCache(sqlite3* db) -> ProjectNameCache& {
  auto& cache = ProjectNameCache::Instance();
  cache.EnsureLoaded(db);
  return cache;
}

template <typename ReportDataT>
inline void EnsureProjectTree(ReportDataT& data,
                              const IProjectInfoProvider& provider) {
  if (data.total_duration <= 0 || !data.project_tree.empty()) {
    return;
  }
  BuildProjectTreeFromIds(data.project_tree, data.project_stats, provider);
}

template <typename MapT, typename FormatterT, typename InserterT>
inline void FormatReportMap(MapT& data_map, FormatterT& formatter,
                            const IProjectInfoProvider& provider,
                            InserterT insert) {
  for (auto& [key, data] : data_map) {
    if (data.total_duration <= 0) {
      continue;
    }
    EnsureProjectTree(data, provider);
    insert(key, formatter->FormatReport(data));
  }
}

}  // namespace reports::services

#endif  // REPORTS_SERVICES_BATCH_EXPORT_HELPERS_H_
