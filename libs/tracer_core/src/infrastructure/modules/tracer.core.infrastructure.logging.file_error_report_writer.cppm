module;

#include <filesystem>

#include "domain/ports/diagnostics.hpp"

export module tracer.core.infrastructure.logging.file_error_report_writer;

export namespace tracer::core::infrastructure::logging {

#include "infrastructure/logging/detail/file_error_report_writer_decl.inc"

}  // namespace tracer::core::infrastructure::logging

export namespace tracer::core::infrastructure::modlogging {

using tracer::core::infrastructure::logging::FileErrorReportWriter;

}  // namespace tracer::core::infrastructure::modlogging
