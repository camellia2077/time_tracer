// infrastructure/logging/file_error_report_writer.hpp
#ifndef INFRASTRUCTURE_LOGGING_FILE_ERROR_REPORT_WRITER_H_
#define INFRASTRUCTURE_LOGGING_FILE_ERROR_REPORT_WRITER_H_

#include <filesystem>

#include "domain/ports/diagnostics.hpp"

namespace tracer::core::infrastructure::logging {

#include "infrastructure/logging/detail/file_error_report_writer_decl.inc"

}  // namespace tracer::core::infrastructure::logging

namespace infrastructure::logging {

using tracer::core::infrastructure::logging::FileErrorReportWriter;

}  // namespace infrastructure::logging

#endif  // INFRASTRUCTURE_LOGGING_FILE_ERROR_REPORT_WRITER_H_
