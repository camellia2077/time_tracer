#include "infra/crypto/tracer_exchange_service_internal.hpp"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <ios>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

import tracer.core.infrastructure.crypto.exchange;
import tracer.core.infrastructure.config.loader.converter_config_loader;
import tracer.core.shared.canonical_text;

namespace tracer_core::infrastructure::crypto::tracer_exchange_internal {

namespace infra_config = tracer::core::infrastructure::config;
namespace modtext = tracer::core::shared::canonical_text;

namespace {

auto CopyFileWithOverwrite(const fs::path& source, const fs::path& target)
    -> void {
  EnsureParentDirectory(target);
  std::error_code error;
  fs::copy_file(source, target, fs::copy_options::overwrite_existing, error);
  if (error) {
    throw std::runtime_error("Failed to copy file: " + source.string() +
                             " -> " + target.string() + " | " +
                             error.message());
  }
}

auto BuildBackupPath(const fs::path& backup_root, std::string_view relative_path)
    -> fs::path {
  return backup_root / fs::path(relative_path);
}

auto ToFileCryptoSecurityLevel(
    app_dto::TracerExchangeSecurityLevel security_level)
    -> file_crypto::FileCryptoSecurityLevel {
  switch (security_level) {
    case app_dto::TracerExchangeSecurityLevel::kMin:
      return file_crypto::FileCryptoSecurityLevel::kMin;
    case app_dto::TracerExchangeSecurityLevel::kInteractive:
      return file_crypto::FileCryptoSecurityLevel::kInteractive;
    case app_dto::TracerExchangeSecurityLevel::kModerate:
      return file_crypto::FileCryptoSecurityLevel::kModerate;
    case app_dto::TracerExchangeSecurityLevel::kHigh:
      return file_crypto::FileCryptoSecurityLevel::kHigh;
    case app_dto::TracerExchangeSecurityLevel::kMax:
      return file_crypto::FileCryptoSecurityLevel::kMax;
  }
  return file_crypto::FileCryptoSecurityLevel::kInteractive;
}

auto ToProgressPhaseString(file_crypto::FileCryptoPhase phase) -> std::string {
  switch (phase) {
    case file_crypto::FileCryptoPhase::kScan:
      return "scan";
    case file_crypto::FileCryptoPhase::kReadInput:
      return "read_input";
    case file_crypto::FileCryptoPhase::kCompress:
      return "compress";
    case file_crypto::FileCryptoPhase::kDeriveKey:
      return "derive_key";
    case file_crypto::FileCryptoPhase::kEncrypt:
      return "encrypt";
    case file_crypto::FileCryptoPhase::kDecrypt:
      return "decrypt";
    case file_crypto::FileCryptoPhase::kDecompress:
      return "decompress";
    case file_crypto::FileCryptoPhase::kWriteOutput:
      return "write_output";
    case file_crypto::FileCryptoPhase::kCompleted:
      return "completed";
    case file_crypto::FileCryptoPhase::kCancelled:
      return "cancelled";
    case file_crypto::FileCryptoPhase::kFailed:
      return "failed";
  }
  return "unknown";
}

auto ToTracerExchangeProgressSnapshot(
    const file_crypto::FileCryptoProgressSnapshot& snapshot)
    -> app_dto::TracerExchangeProgressSnapshot {
  return {
      .input_root_path = snapshot.input_root_path,
      .output_root_path = snapshot.output_root_path,
      .current_input_path = snapshot.current_input_path,
      .current_output_path = snapshot.current_output_path,
      .current_group_label = snapshot.current_group_label,
      .group_index = snapshot.group_index,
      .group_count = snapshot.group_count,
      .file_index_in_group = snapshot.file_index_in_group,
      .file_count_in_group = snapshot.file_count_in_group,
      .current_file_index = snapshot.current_file_index,
      .total_files = snapshot.total_files,
      .current_file_done_bytes = snapshot.current_file_done_bytes,
      .current_file_total_bytes = snapshot.current_file_total_bytes,
      .overall_done_bytes = snapshot.overall_done_bytes,
      .overall_total_bytes = snapshot.overall_total_bytes,
      .speed_bytes_per_sec = snapshot.speed_bytes_per_sec,
      .remaining_bytes = snapshot.remaining_bytes,
      .eta_seconds = snapshot.eta_seconds,
      .is_encrypt_operation =
          snapshot.operation == file_crypto::FileCryptoOperation::kEncrypt,
      .phase = ToProgressPhaseString(snapshot.phase),
  };
}

}  // namespace

auto ToLowerAscii(std::string value) -> std::string {
  std::transform(value.begin(), value.end(), value.begin(),
                 [](unsigned char ch) -> char {
                   return static_cast<char>(std::tolower(ch));
                 });
  return value;
}

auto HasExtensionCaseInsensitive(const fs::path& path,
                                 std::string_view ext_lower) -> bool {
  return ToLowerAscii(path.extension().string()) == ext_lower;
}

auto ParseMonthInfoFromFileName(std::string_view file_name,
                                std::string_view source_label)
    -> ParsedMonthInfo {
  if (file_name.size() != 11U || file_name[4] != '-' ||
      file_name.substr(7U) != ".txt") {
    throw std::runtime_error(
        "Expected canonical month TXT file name YYYY-MM.txt: " +
        std::string(source_label));
  }
  for (const std::size_t index :
       {std::size_t{0}, std::size_t{1}, std::size_t{2}, std::size_t{3},
        std::size_t{5}, std::size_t{6}}) {
    const char ch = file_name[index];
    if (ch < '0' || ch > '9') {
      throw std::runtime_error(
          "Expected canonical month TXT file name YYYY-MM.txt: " +
          std::string(source_label));
    }
  }

  const int year = std::stoi(std::string(file_name.substr(0U, 4U)));
  const int month = std::stoi(std::string(file_name.substr(5U, 2U)));
  if (month < 1 || month > 12) {
    throw std::runtime_error("Invalid month file name: " +
                             std::string(source_label));
  }

  ParsedMonthInfo result{};
  result.year = year;
  result.month = month;
  result.month_key = std::string(file_name.substr(0U, 7U));
  result.file_name = std::string(file_name);
  return result;
}

auto ParseMonthInfoFromCanonicalText(std::span<const std::uint8_t> bytes,
                                     std::string_view source_label)
    -> ParsedMonthInfo {
  const std::string text =
      modtext::RequireCanonicalText(bytes, source_label);
  std::optional<int> year;
  std::optional<int> month;
  std::istringstream stream(text);
  std::string line;
  while (std::getline(stream, line)) {
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }
    const std::string_view trimmed(line);
    if (trimmed.empty()) {
      continue;
    }
    if (!year.has_value() && trimmed.size() == 5U && trimmed.front() == 'y' &&
        std::all_of(trimmed.begin() + 1U, trimmed.end(), [](char ch) {
          return ch >= '0' && ch <= '9';
        })) {
      year = std::stoi(std::string(trimmed.substr(1U)));
      continue;
    }
    if (year.has_value() && !month.has_value() && trimmed.size() == 3U &&
        trimmed.front() == 'm' &&
        std::all_of(trimmed.begin() + 1U, trimmed.end(), [](char ch) {
          return ch >= '0' && ch <= '9';
        })) {
      month = std::stoi(std::string(trimmed.substr(1U)));
      if (*month < 1 || *month > 12) {
        throw std::runtime_error("Invalid month header in " +
                                 std::string(source_label));
      }
      break;
    }
  }

  if (!year.has_value() || !month.has_value()) {
    throw std::runtime_error(
        "TXT payload requires canonical month headers yYYYY + mMM: " +
        std::string(source_label));
  }

  ParsedMonthInfo result{};
  result.year = *year;
  result.month = *month;
  std::ostringstream month_key_stream;
  month_key_stream << std::setw(4) << std::setfill('0') << result.year << "-"
                   << std::setw(2) << std::setfill('0') << result.month;
  result.month_key = month_key_stream.str();
  result.file_name = result.month_key + ".txt";
  return result;
}

auto SanitizeStem(std::string value) -> std::string {
  if (value.empty()) {
    return "tracer_exchange";
  }
  for (char& ch : value) {
    const unsigned char raw = static_cast<unsigned char>(ch);
    const bool keep =
        std::isalnum(raw) != 0 || ch == '.' || ch == '-' || ch == '_';
    if (!keep) {
      ch = '_';
    }
  }
  return value;
}

auto BuildUniqueSuffix() -> std::string {
  const auto now = std::chrono::system_clock::now().time_since_epoch();
  const auto micros =
      std::chrono::duration_cast<std::chrono::microseconds>(now).count();
  return std::to_string(micros);
}

auto BuildScopedStagingDir(const fs::path& writable_root,
                           std::string_view purpose, std::string_view stem)
    -> fs::path {
  const fs::path base_root =
      writable_root.empty() ? fs::current_path() : fs::absolute(writable_root);
  return base_root / ".tracer_staging" / std::string(purpose) /
         (SanitizeStem(std::string(stem)) + "-" + BuildUniqueSuffix());
}

auto EnsureDirectoryRemoved(const fs::path& path) -> void {
  std::error_code error;
  fs::remove_all(path, error);
  if (error) {
    throw std::runtime_error("Failed to remove directory: " + path.string() +
                             " | " + error.message());
  }
}

auto RemoveDirectoryBestEffort(const fs::path& path) -> void {
  std::error_code error;
  fs::remove_all(path, error);
}

auto EnsureParentDirectory(const fs::path& path) -> void {
  const fs::path parent = path.parent_path();
  if (parent.empty()) {
    return;
  }
  std::error_code error;
  fs::create_directories(parent, error);
  if (error) {
    throw std::runtime_error("Failed to create parent directory: " +
                             parent.string() + " | " + error.message());
  }
}

auto ReadFileBytes(const fs::path& path) -> std::vector<std::uint8_t> {
  std::ifstream input(path, std::ios::binary);
  if (!input.is_open()) {
    throw std::runtime_error("Failed to open file for reading: " +
                             path.string());
  }
  return {std::istreambuf_iterator<char>(input),
          std::istreambuf_iterator<char>()};
}

auto WriteFileBytes(const fs::path& path, std::span<const std::uint8_t> bytes)
    -> void {
  EnsureParentDirectory(path);
  std::ofstream output(path, std::ios::binary | std::ios::trunc);
  if (!output.is_open()) {
    throw std::runtime_error("Failed to open file for writing: " +
                             path.string());
  }
  if (!bytes.empty()) {
    output.write(reinterpret_cast<const char*>(bytes.data()),
                 static_cast<std::streamsize>(bytes.size()));
  }
  if (!output.good()) {
    throw std::runtime_error("Failed to write file: " + path.string());
  }
}

auto IsCanonicalTextPackagePath(std::string_view relative_path) -> bool {
  if (relative_path == exchange_pkg::kManifestPath) {
    return true;
  }
  const std::string extension =
      ToLowerAscii(fs::path(relative_path).extension().string());
  return extension == ".txt" || extension == ".toml";
}

auto CanonicalizePackageTextBytes(std::span<const std::uint8_t> bytes,
                                  std::string_view source_label)
    -> std::vector<std::uint8_t> {
  return modtext::ToUtf8Bytes(
      modtext::RequireCanonicalText(bytes, source_label));
}

auto CanonicalizePackageTextBytes(std::string_view text,
                                  std::string_view source_label)
    -> std::vector<std::uint8_t> {
  return modtext::ToUtf8Bytes(
      modtext::RequireCanonicalText(text, source_label));
}

auto EnsureRegularFileExists(const fs::path& path, std::string_view label)
    -> void {
  if (!fs::exists(path) || !fs::is_regular_file(path)) {
    throw std::runtime_error(std::string(label) + " is missing: " +
                             path.string());
  }
}

auto EnsureCryptoResultOk(const file_crypto::FileCryptoResult& result,
                          std::string_view action,
                          const fs::path& input_path) -> void {
  if (result.ok()) {
    return;
  }
  throw std::runtime_error(std::string(action) + " failed for " +
                           input_path.string() + ": " + result.error_code +
                           " | " + result.error_message);
}

auto ResolveActiveConverterConfigPaths(
    const fs::path& active_converter_main_config_path)
    -> ActiveConverterConfigPaths {
  if (active_converter_main_config_path.empty()) {
    throw std::invalid_argument(
        "active_converter_main_config_path must not be empty.");
  }
  const fs::path main_config_path =
      fs::absolute(active_converter_main_config_path);
  const fs::path converter_dir = main_config_path.parent_path();
  return {
      .main_config_path = main_config_path,
      .alias_mapping_path = converter_dir / "alias_mapping.toml",
      .duration_rules_path = converter_dir / "duration_rules.toml",
  };
}

auto EnsureActiveConverterConfigExists(
    const ActiveConverterConfigPaths& active_paths) -> void {
  EnsureRegularFileExists(active_paths.main_config_path,
                          "Active converter main config");
  EnsureRegularFileExists(active_paths.alias_mapping_path,
                          "Active alias mapping config");
  EnsureRegularFileExists(active_paths.duration_rules_path,
                          "Active duration rules config");
}

auto BackupActiveConverterConfig(const ActiveConverterConfigPaths& active_paths,
                                 const fs::path& backup_root) -> void {
  EnsureDirectoryRemoved(backup_root);
  EnsureActiveConverterConfigExists(active_paths);
  CopyFileWithOverwrite(active_paths.main_config_path,
                        BuildBackupPath(backup_root, exchange_pkg::kConverterMainPath));
  CopyFileWithOverwrite(active_paths.alias_mapping_path,
                        BuildBackupPath(backup_root, exchange_pkg::kAliasMappingPath));
  CopyFileWithOverwrite(active_paths.duration_rules_path,
                        BuildBackupPath(backup_root, exchange_pkg::kDurationRulesPath));
}

auto RestoreBackupConfig(const fs::path& backup_root,
                         const ActiveConverterConfigPaths& active_paths)
    -> void {
  CopyFileWithOverwrite(
      BuildBackupPath(backup_root, exchange_pkg::kConverterMainPath),
      active_paths.main_config_path);
  CopyFileWithOverwrite(
      BuildBackupPath(backup_root, exchange_pkg::kAliasMappingPath),
      active_paths.alias_mapping_path);
  CopyFileWithOverwrite(
      BuildBackupPath(backup_root, exchange_pkg::kDurationRulesPath),
      active_paths.duration_rules_path);
}

auto ValidatePackageConverterConfig(const fs::path& work_root) -> void {
  const fs::path main_config_path =
      work_root / fs::path(exchange_pkg::kConverterMainPath);
  EnsureRegularFileExists(main_config_path, "Package converter main config");
  static_cast<void>(
      infra_config::ConverterConfigLoader::LoadFromFile(main_config_path));
}

auto ApplyPackageConverterConfig(const fs::path& work_root,
                                 const ActiveConverterConfigPaths& active_paths)
    -> void {
  CopyFileWithOverwrite(work_root / fs::path(exchange_pkg::kConverterMainPath),
                        active_paths.main_config_path);
  CopyFileWithOverwrite(work_root / fs::path(exchange_pkg::kAliasMappingPath),
                        active_paths.alias_mapping_path);
  CopyFileWithOverwrite(work_root / fs::path(exchange_pkg::kDurationRulesPath),
                        active_paths.duration_rules_path);
}

auto BuildCryptoOptions(
    app_dto::TracerExchangeSecurityLevel security_level,
    const app_dto::TracerExchangeProgressObserver& progress_observer)
    -> file_crypto::FileCryptoOptions {
  file_crypto::FileCryptoOptions options{};
  options.security_level = ToFileCryptoSecurityLevel(security_level);
  if (progress_observer) {
    options.progress_callback =
        [progress_observer](
            const file_crypto::FileCryptoProgressSnapshot& snapshot) {
          try {
            return progress_observer(ToTracerExchangeProgressSnapshot(snapshot)) ==
                           app_dto::TracerExchangeProgressControl::kCancel
                       ? file_crypto::FileCryptoControl::kCancel
                       : file_crypto::FileCryptoControl::kContinue;
          } catch (...) {
            return file_crypto::FileCryptoControl::kContinue;
          }
        };
  }
  return options;
}

auto WriteDecodedPackageToRoot(const exchange_pkg::DecodedTracerExchangePackage& package,
                               const fs::path& root) -> void {
  for (const auto& entry : package.entries) {
    const fs::path target = root / fs::path(entry.relative_path);
    if (IsCanonicalTextPackagePath(entry.relative_path)) {
      const auto canonical_bytes =
          CanonicalizePackageTextBytes(entry.data, target.string());
      WriteFileBytes(target, canonical_bytes);
      continue;
    }
    WriteFileBytes(target, entry.data);
  }
}

}  // namespace tracer_core::infrastructure::crypto::tracer_exchange_internal
