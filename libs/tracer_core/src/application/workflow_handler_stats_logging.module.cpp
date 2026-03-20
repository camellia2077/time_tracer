#include <filesystem>
#include <iomanip>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "application/workflow_handler.hpp"
#include "application/ports/i_processed_data_loader.hpp"
#include "application/ports/logger.hpp"

namespace fs = std::filesystem;

struct AppOptions;
struct DailyLog;

namespace tracer_core::application::ports {
class IConverterConfigProvider;
class IDatabaseHealthChecker;
class IIngestInputProvider;
class IProcessedDataLoader;
class IProcessedDataStorage;
class ITimeSheetRepository;
class IValidationIssueReporter;
}  // namespace tracer_core::application::ports

import tracer.core.application.importer.service;
import tracer.core.domain.model.daily_log;

namespace app_ports = tracer_core::application::ports;
using tracer::core::application::modimporter::ImportService;
using tracer::core::application::modimporter::ImportStats;
using tracer::core::application::modimporter::ReplaceAllTarget;
using tracer::core::application::modimporter::ReplaceMonthTarget;
using tracer::core::domain::model::DailyLog;

namespace workflow_handler_internal {
auto ThrowIfImportTaskFailed(const ImportStats& stats,
                             std::string_view default_message) -> void;
}  // namespace workflow_handler_internal

#include "application/internal/workflow_handler_stats_logging_impl.inc"
