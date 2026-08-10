// infra/insights/export_utils.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_EXPORT_UTILS_H_
#define INFRASTRUCTURE_INSIGHTS_EXPORT_UTILS_H_

// core/ExportUtils.hpp

#include <filesystem>
#include <functional>
#include <optional>
#include <string>

#include "domain/insights/types/insights_types.hpp"

namespace tracer::core::infrastructure::insights {
struct InsightsFormatDetails {
  std::string dir_name;
  std::string extension;
};

[[nodiscard]] auto GetInsightsFormatDetails(InsightsFormat format)
    -> std::optional<InsightsFormatDetails>;

void ExecuteExportTask(const std::string& insights_type_name_singular,
                       const std::filesystem::path& export_root_path,
                       const std::function<int()>& file_writing_lambda);

}  // namespace tracer::core::infrastructure::insights

namespace ExportUtils {

using tracer::core::infrastructure::insights::ExecuteExportTask;
using tracer::core::infrastructure::insights::GetInsightsFormatDetails;
using tracer::core::infrastructure::insights::InsightsFormatDetails;

}  // namespace ExportUtils

#endif  // INFRASTRUCTURE_INSIGHTS_EXPORT_UTILS_H_
