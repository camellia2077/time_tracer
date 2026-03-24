#include <algorithm>
#include <cctype>
#include <filesystem>
#include <format>
#include <fstream>
#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "application/workflow_handler.hpp"
#include "application/ports/pipeline/i_database_health_checker.hpp"
#include "application/ports/pipeline/i_time_sheet_repository.hpp"
#include "application/runtime_bridge/logger.hpp"

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

import tracer.core.application.pipeline.orchestrator;
import tracer.core.application.pipeline.types;
import tracer.core.domain.logic.converter.core;
import tracer.core.domain.logic.converter.log_processor;
import tracer.core.domain.model.daily_log;
import tracer.core.domain.ports.diagnostics;
import tracer.core.domain.types.app_options;
import tracer.core.domain.types.date_check_mode;
import tracer.core.domain.types.ingest_mode;
import tracer.core.shared.canonical_text;
import tracer.core.shared.string_utils;

namespace app_ports = tracer_core::application::ports;
namespace runtime_bridge = tracer_core::application::runtime_bridge;
namespace app_pipeline = tracer::core::application::pipeline;
namespace modtext = tracer::core::shared::canonical_text;
using tracer::core::shared::string_utils::Trim;
using tracer::core::domain::modlogic::converter::LogLinker;
using tracer::core::domain::modlogic::converter::LogProcessor;
using tracer::core::domain::model::DailyLog;
using tracer::core::domain::types::AppOptions;
using tracer::core::domain::types::DateCheckMode;
using tracer::core::domain::types::IngestMode;
namespace modports = tracer::core::domain::ports;

using app_pipeline::PipelineOrchestrator;
using app_pipeline::PipelineSession;

namespace workflow_handler_internal {
[[nodiscard]] auto BuildPipelineFailureMessage(std::string_view base_message)
    -> std::string;
}  // namespace workflow_handler_internal

#include "application/internal/workflow_handler_import_namespace.inc"

#include "application/internal/workflow_handler_import_flow_impl.inc"
