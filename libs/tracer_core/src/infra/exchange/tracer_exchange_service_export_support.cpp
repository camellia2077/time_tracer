#include "infra/exchange/tracer_exchange_service_export_support.hpp"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <map>
#include <sstream>
#include <stdexcept>

import tracer.core.infrastructure.exchange;

namespace tracer_core::infrastructure::crypto::tracer_exchange_internal {

auto ResolveEncryptOutputPath(const ResolveEncryptOutputPathRequest& request)
    -> fs::path {
  fs::path output_path = request.output_arg;
  if (fs::exists(output_path) && fs::is_directory(output_path)) {
    output_path /= request.input_path.filename();
  }
  output_path.replace_extension(".tracer");
  return output_path;
}

auto CurrentUtcTimestampRfc3339() -> std::string {
  const auto kNow = std::chrono::system_clock::now();
  const std::time_t kRawTime = std::chrono::system_clock::to_time_t(kNow);
  std::tm kUtcTime{};
#if defined(_WIN32)
  gmtime_s(&kUtcTime, &kRawTime);
#else
  gmtime_r(&kRawTime, &kUtcTime);
#endif
  std::ostringstream stream;
  stream << std::put_time(&kUtcTime, "%Y-%m-%dT%H:%M:%SZ");
  return stream.str();
}

namespace {

auto ResolvePayloadPackagePath(const fs::path& source_path) -> std::string {
  const ParsedMonthInfo kFileInfo = ParseMonthInfoFromFileName(
      source_path.filename().string(), source_path.string());
  const ParsedMonthInfo kHeaderInfo = ParseMonthInfoFromCanonicalText(
      ReadFileBytes(source_path), source_path.string());
  if (kFileInfo.month_key != kHeaderInfo.month_key) {
    throw std::runtime_error(
        "TXT file name must match canonical month headers yYYYY + mMM: " +
        source_path.string());
  }

  return (fs::path(exchange_pkg::kPayloadRoot) /
          std::to_string(kFileInfo.year) / kFileInfo.file_name)
      .generic_string();
}

auto ResolvePayloadPackagePath(const ParsedMonthInfo& header_info)
    -> std::string {
  return (fs::path(exchange_pkg::kPayloadRoot) /
          std::to_string(header_info.year) / header_info.file_name)
      .generic_string();
}

}  // namespace

auto CollectInputPayloadFilesFromRoot(const fs::path& input_root)
    -> std::vector<InputPayloadFile> {
  std::vector<InputPayloadFile> payload_files;
  std::map<std::string, fs::path> path_by_month;
  for (const auto& entry : fs::recursive_directory_iterator(input_root)) {
    if (!entry.is_regular_file()) {
      continue;
    }
    if (!HasExtensionCaseInsensitive(entry.path(), ".txt")) {
      continue;
    }
    const std::string kRelativePackagePath =
        ResolvePayloadPackagePath(entry.path());
    const ParsedMonthInfo kMonthInfo = ParseMonthInfoFromFileName(
        entry.path().filename().string(), entry.path().string());
    if (const auto [it, inserted] =
            path_by_month.emplace(kMonthInfo.month_key, entry.path());
        !inserted) {
      throw std::runtime_error(
          "Duplicate month TXT inputs detected for " + kMonthInfo.month_key +
          ": " + it->second.string() + " | " + entry.path().string());
    }
    payload_files.push_back(InputPayloadFile{
        .source_path = entry.path(),
        .source_label = entry.path().string(),
        .relative_package_path = kRelativePackagePath,
        .month_key = kMonthInfo.month_key,
        .year = kMonthInfo.year,
        .month = kMonthInfo.month,
    });
  }

  std::sort(payload_files.begin(), payload_files.end(),
            [](const InputPayloadFile& lhs, const InputPayloadFile& rhs) {
              return lhs.relative_package_path < rhs.relative_package_path;
            });
  return payload_files;
}

auto CollectInputPayloadFilesFromPayloads(
    const std::vector<app_dto::TracerExchangeTextPayloadItem>& payload_items)
    -> std::vector<InputPayloadFile> {
  std::vector<InputPayloadFile> payload_files;
  std::map<std::string, std::string> label_by_month;
  payload_files.reserve(payload_items.size());
  for (const auto& payload_item : payload_items) {
    const std::string kSourceLabel = payload_item.relative_path_hint.empty()
                                         ? std::string("(payload_item)")
                                         : payload_item.relative_path_hint;
    const std::vector<std::uint8_t> kContentBytes(payload_item.content.begin(),
                                                  payload_item.content.end());
    const ParsedMonthInfo kHeaderInfo =
        ParseMonthInfoFromCanonicalText(kContentBytes, kSourceLabel);
    const std::string kRelativePackagePath =
        ResolvePayloadPackagePath(kHeaderInfo);
    if (!payload_item.relative_path_hint.empty()) {
      const std::string kNormalizedHint =
          fs::path(payload_item.relative_path_hint).generic_string();
      const std::string kExpectedHint =
          (fs::path(std::to_string(kHeaderInfo.year)) / kHeaderInfo.file_name)
              .generic_string();
      if (kNormalizedHint != kExpectedHint) {
        throw std::runtime_error(
            "TXT payload hint must match canonical month headers yYYYY + mMM: " +
            payload_item.relative_path_hint + " -> expected " + kExpectedHint);
      }
    }
    if (const auto [it, inserted] =
            label_by_month.emplace(kHeaderInfo.month_key, kSourceLabel);
        !inserted) {
      throw std::runtime_error(
          "Duplicate month TXT payloads detected for " + kHeaderInfo.month_key +
          ": " + it->second + " | " + kSourceLabel);
    }

    payload_files.push_back(InputPayloadFile{
        .source_label = kSourceLabel,
        .content_bytes = kContentBytes,
        .relative_package_path = kRelativePackagePath,
        .month_key = kHeaderInfo.month_key,
        .year = kHeaderInfo.year,
        .month = kHeaderInfo.month,
    });
  }

  std::sort(payload_files.begin(), payload_files.end(),
            [](const InputPayloadFile& lhs, const InputPayloadFile& rhs) {
              return lhs.relative_package_path < rhs.relative_package_path;
            });
  return payload_files;
}

auto ValidateInputForExport(
    app_workflow::IWorkflowHandler& workflow_handler,
    tracer::core::domain::types::DateCheckMode date_check_mode,
    const fs::path& input_path) -> void {
  const std::string kInput = input_path.string();
  workflow_handler.RunValidateStructure(kInput);
  workflow_handler.RunValidateLogic(kInput, date_check_mode);
}

auto ValidateInputPayloadsForExport(
    app_workflow::IWorkflowHandler& workflow_handler,
    tracer::core::domain::types::DateCheckMode date_check_mode,
    const std::vector<InputPayloadFile>& payload_files) -> void {
  for (const auto& payload_file : payload_files) {
    if (payload_file.source_path.empty()) {
      continue;
    }
    ValidateInputForExport(workflow_handler, date_check_mode,
                           payload_file.source_path);
  }
}

}  // namespace tracer_core::infrastructure::crypto::tracer_exchange_internal
