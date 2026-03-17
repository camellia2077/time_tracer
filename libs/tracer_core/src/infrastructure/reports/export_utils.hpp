// infrastructure/reports/export_utils.hpp
#ifndef INFRASTRUCTURE_REPORTS_EXPORT_UTILS_H_
#define INFRASTRUCTURE_REPORTS_EXPORT_UTILS_H_

// core/ExportUtils.hpp

#include <filesystem>
#include <functional>
#include <optional>
#include <string>

#include "domain/reports/types/report_types.hpp"

namespace tracer::core::infrastructure::reports {
struct ReportFormatDetails {
  std::string dir_name;
  std::string extension;
};

[[nodiscard]] auto GetReportFormatDetails(ReportFormat format)
    -> std::optional<ReportFormatDetails>;

void ExecuteExportTask(const std::string& report_type_name_singular,
                       const std::filesystem::path& export_root_path,
                       const std::function<int()>& file_writing_lambda);

}  // namespace tracer::core::infrastructure::reports

namespace ExportUtils {

using tracer::core::infrastructure::reports::ExecuteExportTask;
using tracer::core::infrastructure::reports::GetReportFormatDetails;
using tracer::core::infrastructure::reports::ReportFormatDetails;

}  // namespace ExportUtils

#endif  // INFRASTRUCTURE_REPORTS_EXPORT_UTILS_H_
