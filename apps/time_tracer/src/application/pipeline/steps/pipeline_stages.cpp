// application/pipeline/steps/pipeline_stages.cpp
#include "application/pipeline/steps/pipeline_stages.hpp"

#include <chrono>
#include <future>
#include <iomanip>
#include <iterator>
#include <mutex>
#include <set>
#include <sstream>
#include <vector>

#include "application/ports/i_validation_issue_reporter.hpp"
#include "application/ports/logger.hpp"
#include "domain/logic/converter/convert/core/converter_core.hpp"
#include "domain/logic/converter/log_processor.hpp"
#include "domain/logic/validator/common/validator_utils.hpp"
#include "domain/logic/validator/structure/structure_validator.hpp"
#include "domain/logic/validator/txt/facade/text_validator.hpp"
#include "shared/types/ansi_colors.hpp"

namespace core::pipeline {

namespace {
constexpr double kMillisPerSecond = 1000.0;

auto ResolveLogicFallbackLabel(const std::string& month_key,
                               const std::vector<DailyLog>& days)
    -> std::string {
  for (const auto& day : days) {
    if (day.source_span.has_value() && !day.source_span->file_path.empty()) {
      return day.source_span->file_path;
    }
  }

  for (const auto& day : days) {
    for (const auto& activity : day.processedActivities) {
      if (activity.source_span.has_value() &&
          !activity.source_span->file_path.empty()) {
        return activity.source_span->file_path;
      }
    }
  }

  return "ProcessedData[" + month_key + "]";
}
}  // namespace

auto FileCollector::Execute(
    PipelineContext& context,
    const time_tracer::application::ports::IIngestInputProvider& input_provider,
    const std::string& extension) -> bool {
  context.state.ingest_inputs.clear();
  context.state.generated_files.clear();

  const auto kInputCollection =
      input_provider.CollectTextInputs(context.config.input_root, extension);

  if (!kInputCollection.input_exists) {
    time_tracer::application::ports::LogError(
        std::string(time_tracer::common::colors::kRed) +
        "错误: 输入的路径不存在: " + context.config.input_root.string() +
        std::string(time_tracer::common::colors::kReset));
    return false;
  }

  if (kInputCollection.inputs.empty()) {
    time_tracer::application::ports::LogWarn(
        std::string(time_tracer::common::colors::kYellow) +
        "警告: 在指定路径下没有找到 " + extension + " 文件。" +
        std::string(time_tracer::common::colors::kReset));
    return false;
  }

  context.state.ingest_inputs = kInputCollection.inputs;
  time_tracer::application::ports::LogInfo(
      "信息: 成功收集到 " + std::to_string(context.state.ingest_inputs.size()) +
      " 个待处理文件 (" + extension + ").");
  return true;
}

auto StructureValidatorStep::Execute(PipelineContext& context) -> bool {
  time_tracer::application::ports::LogInfo(
      "Step: Validating Source Structure (TXT)...");

  validator::txt::TextValidator validator(context.state.converter_config);

  bool all_valid = true;
  int files_checked = 0;

  for (const auto& input : context.state.ingest_inputs) {
    files_checked++;
    const std::string kSourcePath =
        input.source_id.empty() ? input.source_label : input.source_id;
    const std::string kDisplayLabel =
        input.source_label.empty() ? kSourcePath : input.source_label;

    std::set<validator::Error> errors;

    if (!validator.Validate(kSourcePath, input.content, errors)) {
      all_valid = false;
      if (context.state.validation_issue_reporter != nullptr) {
        context.state.validation_issue_reporter->ReportStructureErrors(
            kDisplayLabel, errors);
      }
    }
  }

  if (all_valid) {
    time_tracer::application::ports::LogInfo(
        std::string(time_tracer::common::colors::kGreen) +
        "Structure validation passed for " + std::to_string(files_checked) +
        " files." + std::string(time_tracer::common::colors::kReset));
  } else {
    time_tracer::application::ports::LogError(
        std::string(time_tracer::common::colors::kRed) +
        "Structure validation failed. Please fix the errors above." +
        std::string(time_tracer::common::colors::kReset));
  }

  return all_valid;
}

auto ConverterStep::Execute(PipelineContext& context) -> bool {
  time_tracer::application::ports::LogInfo(
      "Step: Converting files (Parallel)...");
  auto start_time = std::chrono::steady_clock::now();

  std::mutex data_mutex;
  std::vector<std::future<LogProcessingResult>> futures;

  for (const auto& input : context.state.ingest_inputs) {
    futures.push_back(std::async(
        std::launch::async, [&context, input]() -> LogProcessingResult {
          try {
            LogProcessor processor(context.state.converter_config);
            return processor.ProcessSourceContent(input.source_id,
                                                  input.content);

          } catch (const std::exception& e) {
            const std::string kSourceLabel = input.source_label.empty()
                                                 ? input.source_id
                                                 : input.source_label;
            time_tracer::application::ports::LogError(
                std::string(time_tracer::common::colors::kRed) +
                "Thread Error [" + kSourceLabel + "]: " + e.what() +
                std::string(time_tracer::common::colors::kReset));
            return LogProcessingResult{.success = false, .processed_data = {}};
          }
        }));
  }

  bool all_success = true;
  int processed_count = 0;

  for (auto& future : futures) {
    LogProcessingResult result = future.get();
    processed_count++;

    if (!result.success) {
      all_success = false;
    } else {
      std::scoped_lock lock(data_mutex);
      // `processed_data` key semantics: year-month bucket (`YYYY-MM`).
      for (auto& [year_month_key, month_days] : result.processed_data) {
        auto& merged_days = context.result.processed_data[year_month_key];
        merged_days.insert(merged_days.end(),
                           std::make_move_iterator(month_days.begin()),
                           std::make_move_iterator(month_days.end()));
      }
    }
  }

  auto end_time = std::chrono::steady_clock::now();
  double duration =
      std::chrono::duration<double, std::milli>(end_time - start_time).count();
  PrintTiming(duration);

  if (all_success) {
    time_tracer::application::ports::LogInfo(
        std::string(time_tracer::common::colors::kGreen) +
        "内存转换阶段 全部成功 (" + std::to_string(processed_count) +
        " files)." + std::string(time_tracer::common::colors::kReset));
  } else {
    time_tracer::application::ports::LogWarn(
        std::string(time_tracer::common::colors::kYellow) +
        "内存转换阶段 完成，但存在部分错误。" +
        std::string(time_tracer::common::colors::kReset));
  }

  return all_success;
}

void ConverterStep::PrintTiming(double total_time_ms) {
  double total_time_s = total_time_ms / kMillisPerSecond;

  std::ostringstream stream;
  stream << "--------------------------------------\n";
  stream << "转换耗时: " << std::fixed << std::setprecision(3) << total_time_s
         << " 秒 (" << total_time_ms << " ms)\n";
  stream << "--------------------------------------";

  time_tracer::application::ports::LogInfo(stream.str());
}

auto LogicLinkerStep::Execute(PipelineContext& context) -> bool {
  if (context.result.processed_data.empty()) {
    return true;
  }

  time_tracer::application::ports::LogInfo("Step: Linking cross-month data...");

  try {
    LogLinker linker(context.state.converter_config);
    linker.LinkLogs(context.result.processed_data);

  } catch (const std::exception& e) {
    time_tracer::application::ports::LogError(
        std::string(time_tracer::common::colors::kRed) +
        "[Pipeline] Logic Linker Error: " + e.what() +
        std::string(time_tracer::common::colors::kReset));
    return true;
  }

  return true;
}

auto LogicValidatorStep::Execute(PipelineContext& context) -> bool {
  time_tracer::application::ports::LogInfo(
      "Step: Validating Business Logic (Dates, Continuity)...");

  if (context.result.processed_data.empty()) {
    time_tracer::application::ports::LogWarn(
        std::string(time_tracer::common::colors::kYellow) +
        "No data to validate." +
        std::string(time_tracer::common::colors::kReset));
    return true;
  }

  validator::structure::StructValidator validator(
      context.config.date_check_mode);

  bool all_valid = true;

  for (const auto& [month_key, days] : context.result.processed_data) {
    if (days.empty()) {
      continue;
    }

    const std::string kFallbackLabel =
        ResolveLogicFallbackLabel(month_key, days);

    std::vector<validator::Diagnostic> diagnostics;
    if (!validator.Validate(kFallbackLabel, days, diagnostics)) {
      all_valid = false;
      if (context.state.validation_issue_reporter != nullptr) {
        context.state.validation_issue_reporter->ReportLogicDiagnostics(
            kFallbackLabel, diagnostics);
      }
    }
  }

  if (all_valid) {
    time_tracer::application::ports::LogInfo(
        std::string(time_tracer::common::colors::kGreen) +
        "Logic validation passed." +
        std::string(time_tracer::common::colors::kReset));
  } else {
    time_tracer::application::ports::LogError(
        std::string(time_tracer::common::colors::kRed) +
        "Logic validation found issues (e.g., broken date continuity)." +
        std::string(time_tracer::common::colors::kReset));
  }

  return all_valid;
}

}  // namespace core::pipeline
