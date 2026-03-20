#include "infrastructure/crypto/tracer_exchange_service_internal.hpp"

#include <algorithm>
#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

import tracer.core.infrastructure.crypto.exchange;

namespace tracer_core::infrastructure::crypto::tracer_exchange_internal {

namespace {

using exchange_pkg::DecodePackageBytes;

auto CollectImportedPayloadFiles(const fs::path& extracted_root)
    -> std::vector<ImportedPayloadFile> {
  const fs::path payload_root = extracted_root / fs::path(exchange_pkg::kPayloadRoot);
  if (!fs::exists(payload_root) || !fs::is_directory(payload_root)) {
    throw std::runtime_error(
        "Decoded tracer exchange package is missing payload root: " +
        payload_root.string());
  }

  std::vector<ImportedPayloadFile> imported_payloads;
  std::map<std::string, fs::path> source_by_month;
  for (const auto& entry : fs::recursive_directory_iterator(payload_root)) {
    if (!entry.is_regular_file() ||
        !HasExtensionCaseInsensitive(entry.path(), ".txt")) {
      continue;
    }

    const fs::path relative_path = entry.path().lexically_relative(extracted_root);
    const std::string relative_text = relative_path.generic_string();
    if (relative_text.empty()) {
      throw std::runtime_error("Failed to resolve imported payload path: " +
                               entry.path().string());
    }

    const ParsedMonthInfo file_info = ParseMonthInfoFromFileName(
        entry.path().filename().string(), entry.path().string());
    const ParsedMonthInfo header_info = ParseMonthInfoFromCanonicalText(
        ReadFileBytes(entry.path()), entry.path().string());
    if (file_info.month_key != header_info.month_key) {
      throw std::runtime_error(
          "Imported payload file name must match canonical month headers yYYYY + mMM: " +
          entry.path().string());
    }
    const std::string expected_relative =
        (fs::path(exchange_pkg::kPayloadRoot) / std::to_string(file_info.year) /
         file_info.file_name)
            .generic_string();
    if (relative_text != expected_relative) {
      throw std::runtime_error("Imported payload path must be exactly " +
                               expected_relative + ": " + entry.path().string());
    }
    if (const auto [it, inserted] =
            source_by_month.emplace(file_info.month_key, entry.path());
        !inserted) {
      throw std::runtime_error("Imported package contains duplicate month payload " +
                               file_info.month_key + ": " +
                               it->second.string() + " | " +
                               entry.path().string());
    }

    imported_payloads.push_back(ImportedPayloadFile{
        .source_path = entry.path(),
        .relative_package_path = relative_text,
        .month_key = file_info.month_key,
        .year = file_info.year,
        .month = file_info.month,
    });
  }

  std::sort(imported_payloads.begin(), imported_payloads.end(),
            [](const ImportedPayloadFile& lhs, const ImportedPayloadFile& rhs) {
              return lhs.month_key < rhs.month_key;
            });
  return imported_payloads;
}

auto CollectManagedMonthFiles(const fs::path& active_text_root)
    -> std::map<std::string, fs::path> {
  std::map<std::string, fs::path> managed_month_files;
  if (active_text_root.empty() || !fs::exists(active_text_root)) {
    return managed_month_files;
  }
  if (!fs::is_directory(active_text_root)) {
    throw std::runtime_error("active_text_root_path must be a directory: " +
                             active_text_root.string());
  }

  for (const auto& entry : fs::recursive_directory_iterator(active_text_root)) {
    if (!entry.is_regular_file()) {
      continue;
    }
    if (!HasExtensionCaseInsensitive(entry.path(), ".txt")) {
      continue;
    }
    const ParsedMonthInfo month_info = ParseMonthInfoFromFileName(
        entry.path().filename().string(), entry.path().string());
    if (const auto [it, inserted] =
            managed_month_files.emplace(month_info.month_key, entry.path());
        !inserted) {
      throw std::runtime_error(
          "active_text_root_path contains duplicate month TXT files for " +
          month_info.month_key + ": " + it->second.string() + " | " +
          entry.path().string());
    }
  }
  return managed_month_files;
}

auto BackupManagedTextFiles(
    const std::vector<ImportedPayloadFile>& imported_payloads,
    const std::map<std::string, fs::path>& managed_month_files,
    const fs::path& backup_root) -> std::vector<std::string> {
  std::vector<std::string> added_month_keys;
  for (const auto& imported_payload : imported_payloads) {
    const auto existing_it = managed_month_files.find(imported_payload.month_key);
    if (existing_it == managed_month_files.end()) {
      added_month_keys.push_back(imported_payload.month_key);
      continue;
    }

    const fs::path backup_path =
        backup_root / fs::path(imported_payload.month_key + ".txt");
    EnsureParentDirectory(backup_path);
    std::error_code error;
    fs::copy_file(existing_it->second, backup_path,
                  fs::copy_options::overwrite_existing, error);
    if (error) {
      throw std::runtime_error("Failed to backup managed text file: " +
                               existing_it->second.string() + " -> " +
                               backup_path.string() + " | " +
                               error.message());
    }
  }

  std::sort(added_month_keys.begin(), added_month_keys.end());
  return added_month_keys;
}

auto RestoreManagedTextFiles(const fs::path& active_text_root,
                             const fs::path& backup_root,
                             const std::vector<std::string>& added_month_keys)
    -> void {
  if (fs::exists(backup_root)) {
    for (const auto& entry : fs::directory_iterator(backup_root)) {
      if (!entry.is_regular_file()) {
        continue;
      }
      EnsureParentDirectory(active_text_root / entry.path().filename());
      std::error_code error;
      fs::copy_file(entry.path(), active_text_root / entry.path().filename(),
                    fs::copy_options::overwrite_existing, error);
      if (error) {
        throw std::runtime_error("Failed to restore managed text file: " +
                                 entry.path().string() + " | " +
                                 error.message());
      }
    }
  }

  for (const auto& added_month_key : added_month_keys) {
    std::error_code error;
    fs::remove(active_text_root / fs::path(added_month_key + ".txt"), error);
    if (error) {
      throw std::runtime_error(
          "Failed to remove newly added month TXT during rollback: " +
          (active_text_root / fs::path(added_month_key + ".txt")).string() +
          " | " + error.message());
    }
  }
}

auto BuildEffectiveTextRoot(
    const fs::path& active_text_root,
    const std::map<std::string, fs::path>& managed_month_files,
    const std::vector<ImportedPayloadFile>& imported_payloads,
    const fs::path& effective_root) -> void {
  RemoveDirectoryBestEffort(effective_root);
  std::error_code error;
  fs::create_directories(effective_root, error);
  if (error) {
    throw std::runtime_error("Failed to create effective text root: " +
                             effective_root.string() + " | " +
                             error.message());
  }

  for (const auto& [month_key, source_path] : managed_month_files) {
    EnsureParentDirectory(effective_root / fs::path(month_key + ".txt"));
    std::error_code copy_error;
    fs::copy_file(source_path, effective_root / fs::path(month_key + ".txt"),
                  fs::copy_options::overwrite_existing, copy_error);
    if (copy_error) {
      throw std::runtime_error("Failed to seed effective text root from active text root: " +
                               source_path.string() + " | " +
                               copy_error.message());
    }
  }

  for (const auto& imported_payload : imported_payloads) {
    EnsureParentDirectory(effective_root / fs::path(imported_payload.month_key + ".txt"));
    std::error_code copy_error;
    fs::copy_file(imported_payload.source_path,
                  effective_root / fs::path(imported_payload.month_key + ".txt"),
                  fs::copy_options::overwrite_existing, copy_error);
    if (copy_error) {
      throw std::runtime_error("Failed to write imported payload into effective text root: " +
                               imported_payload.source_path.string() + " | " +
                               copy_error.message());
    }
  }
}

auto WriteImportedPayloadsToActiveTextRoot(
    const fs::path& active_text_root,
    const std::vector<ImportedPayloadFile>& imported_payloads) -> void {
  std::error_code error;
  fs::create_directories(active_text_root, error);
  if (error) {
    throw std::runtime_error("Failed to create active_text_root_path: " +
                             active_text_root.string() + " | " +
                             error.message());
  }

  for (const auto& imported_payload : imported_payloads) {
    const fs::path target =
        active_text_root / fs::path(imported_payload.month_key + ".txt");
    EnsureParentDirectory(target);
    std::error_code copy_error;
    fs::copy_file(imported_payload.source_path, target,
                  fs::copy_options::overwrite_existing, copy_error);
    if (copy_error) {
      throw std::runtime_error("Failed to update active text root month file: " +
                               imported_payload.source_path.string() + " -> " +
                               target.string() + " | " + copy_error.message());
    }
  }
}

auto EmitImportTransactionProgress(
    const app_dto::TracerExchangeProgressObserver& progress_observer,
    std::string_view phase,
    std::size_t phase_index,
    std::size_t phase_count,
    std::string_view current_item,
    std::size_t done_count,
    std::size_t total_count,
    const fs::path& input_root_path,
    const fs::path& output_root_path,
    const fs::path& current_input_path = {},
    const fs::path& current_output_path = {}) -> void {
  if (!progress_observer) {
    return;
  }

  app_dto::TracerExchangeProgressSnapshot snapshot{};
  snapshot.input_root_path = input_root_path;
  snapshot.output_root_path = output_root_path;
  snapshot.current_input_path = current_input_path;
  snapshot.current_output_path = current_output_path;
  snapshot.current_item = std::string(current_item);
  snapshot.current_group_label = std::string(current_item);
  snapshot.phase_index = phase_index;
  snapshot.phase_count = phase_count;
  snapshot.done_count = done_count;
  snapshot.total_count = total_count;
  snapshot.group_index = phase_index;
  snapshot.group_count = phase_count;
  snapshot.file_index_in_group = done_count;
  snapshot.file_count_in_group = total_count;
  snapshot.current_file_index = done_count;
  snapshot.total_files = total_count;
  snapshot.current_file_done_bytes = done_count;
  snapshot.current_file_total_bytes = total_count;
  snapshot.overall_done_bytes = phase_index;
  snapshot.overall_total_bytes = phase_count;
  snapshot.is_encrypt_operation = false;
  snapshot.phase = std::string(phase);
  if (progress_observer(snapshot) == app_dto::TracerExchangeProgressControl::kCancel) {
    throw std::runtime_error("Tracer exchange import cancelled.");
  }
}

}  // namespace

auto TracerExchangeService::RunImport(
    const app_dto::TracerExchangeImportRequest& request)
    -> app_dto::TracerExchangeImportResult {
  if (request.input_tracer_path.empty()) {
    throw std::invalid_argument("input_path is required.");
  }
  if (request.active_text_root_path.empty()) {
    throw std::invalid_argument("active_text_root_path must not be empty.");
  }
  if (request.active_converter_main_config_path.empty()) {
    throw std::invalid_argument(
        "active_converter_main_config_path must not be empty.");
  }
  if (request.runtime_work_root.empty()) {
    throw std::invalid_argument("runtime_work_root must not be empty.");
  }
  if (request.passphrase.empty()) {
    throw std::invalid_argument("Passphrase must not be empty.");
  }

  const fs::path input_path = fs::absolute(request.input_tracer_path);
  const fs::path active_text_root = fs::absolute(request.active_text_root_path);
  const fs::path runtime_work_root = fs::absolute(request.runtime_work_root);
  if (!fs::exists(input_path) || !fs::is_regular_file(input_path)) {
    throw std::invalid_argument("Decrypt input path must be an existing file: " +
                                input_path.string());
  }
  if (!HasExtensionCaseInsensitive(input_path, ".tracer")) {
    throw std::invalid_argument("Decrypt input file must be .tracer: " +
                                input_path.string());
  }

  const std::string stem =
      input_path.stem().empty() ? input_path.filename().string()
                                : input_path.stem().string();
  const fs::path transaction_root =
      BuildScopedStagingDir(runtime_work_root, "import", stem);
  const fs::path package_path = transaction_root / "exchange.ttpkg";
  const fs::path extracted_root = transaction_root / "decoded";
  const fs::path effective_text_root = transaction_root / "effective_text_root";
  const fs::path backup_root = transaction_root / "backup";
  const fs::path backup_config_root = backup_root / "config";
  const fs::path backup_text_root = backup_root / "text";
  bool config_applied = false;
  bool text_root_updated = false;
  std::vector<std::string> added_month_keys;
  std::string rollback_error_message;
  std::string failure_message;

  std::error_code io_error;
  fs::create_directories(transaction_root, io_error);
  if (io_error) {
    throw std::runtime_error(
        "Failed to create tracer exchange transaction root: " +
        transaction_root.string() + " | " + io_error.message());
  }

  try {
    constexpr std::size_t kPhaseCount = 10U;
    EmitImportTransactionProgress(
        request.progress_observer, "decrypt_package", 1U, kPhaseCount,
        input_path.filename().string(), 0U, 1U, input_path, active_text_root,
        input_path, package_path);
    EnsureCryptoResultOk(
        file_crypto::DecryptFile(
            input_path, package_path, request.passphrase,
            BuildCryptoOptions(app_dto::TracerExchangeSecurityLevel::kInteractive, {})),
        "Decrypt", input_path);
    EmitImportTransactionProgress(
        request.progress_observer, "decrypt_package", 1U, kPhaseCount,
        input_path.filename().string(), 1U, 1U, input_path, active_text_root,
        input_path, package_path);

    EmitImportTransactionProgress(
        request.progress_observer, "validate_package_contract", 2U, kPhaseCount,
        input_path.filename().string(), 0U, 1U, input_path, active_text_root,
        package_path, extracted_root);
    const exchange_pkg::DecodedTracerExchangePackage package =
        DecodePackageBytes(ReadFileBytes(package_path));
    EmitImportTransactionProgress(
        request.progress_observer, "validate_package_contract", 2U, kPhaseCount,
        input_path.filename().string(), 1U, 1U, input_path, active_text_root,
        package_path, extracted_root);

    WriteDecodedPackageToRoot(package, extracted_root);
    EmitImportTransactionProgress(
        request.progress_observer, "validate_converter_config", 3U, kPhaseCount,
        "converter_config", 0U, 1U, input_path, active_text_root,
        extracted_root / fs::path(exchange_pkg::kConverterMainPath),
        request.active_converter_main_config_path);
    ValidatePackageConverterConfig(extracted_root);
    EmitImportTransactionProgress(
        request.progress_observer, "validate_converter_config", 3U, kPhaseCount,
        "converter_config", 1U, 1U, input_path, active_text_root,
        extracted_root / fs::path(exchange_pkg::kConverterMainPath),
        request.active_converter_main_config_path);

    const std::vector<ImportedPayloadFile> imported_payloads =
        CollectImportedPayloadFiles(extracted_root);
    if (imported_payloads.empty()) {
      throw std::runtime_error(
          "Tracer exchange package must contain at least one canonical payload TXT file.");
    }
    const std::map<std::string, fs::path> managed_month_files =
        CollectManagedMonthFiles(active_text_root);

    EmitImportTransactionProgress(
        request.progress_observer, "build_effective_text_view", 4U, kPhaseCount,
        "effective_text_root", 0U,
        managed_month_files.size() + imported_payloads.size(), input_path,
        active_text_root, active_text_root, effective_text_root);
    BuildEffectiveTextRoot(active_text_root, managed_month_files,
                           imported_payloads, effective_text_root);
    EmitImportTransactionProgress(
        request.progress_observer, "build_effective_text_view", 4U, kPhaseCount,
        "effective_text_root",
        managed_month_files.size() + imported_payloads.size(),
        managed_month_files.size() + imported_payloads.size(), input_path,
        active_text_root, active_text_root, effective_text_root);

    const ActiveConverterConfigPaths active_paths =
        ResolveActiveConverterConfigPaths(
            request.active_converter_main_config_path);
    BackupActiveConverterConfig(active_paths, backup_config_root);
    added_month_keys = BackupManagedTextFiles(imported_payloads, managed_month_files,
                                              backup_text_root);

    EmitImportTransactionProgress(
        request.progress_observer, "apply_converter_config", 5U, kPhaseCount,
        "converter_config", 0U, 1U, input_path, active_text_root,
        extracted_root / fs::path(exchange_pkg::kConverterMainPath),
        request.active_converter_main_config_path);
    config_applied = true;
    ApplyPackageConverterConfig(extracted_root, active_paths);
    EmitImportTransactionProgress(
        request.progress_observer, "apply_converter_config", 5U, kPhaseCount,
        "converter_config", 1U, 1U, input_path, active_text_root,
        extracted_root / fs::path(exchange_pkg::kConverterMainPath),
        request.active_converter_main_config_path);

    EmitImportTransactionProgress(
        request.progress_observer, "validate_text_structure", 6U, kPhaseCount,
        effective_text_root.string(), 0U, 1U, input_path, active_text_root,
        effective_text_root, active_text_root);
    workflow_handler_.RunValidateStructure(effective_text_root.string());
    EmitImportTransactionProgress(
        request.progress_observer, "validate_text_structure", 6U, kPhaseCount,
        effective_text_root.string(), 1U, 1U, input_path, active_text_root,
        effective_text_root, active_text_root);

    EmitImportTransactionProgress(
        request.progress_observer, "validate_text_logic", 7U, kPhaseCount,
        effective_text_root.string(), 0U, 1U, input_path, active_text_root,
        effective_text_root, active_text_root);
    workflow_handler_.RunValidateLogic(effective_text_root.string(),
                                       request.date_check_mode);
    EmitImportTransactionProgress(
        request.progress_observer, "validate_text_logic", 7U, kPhaseCount,
        effective_text_root.string(), 1U, 1U, input_path, active_text_root,
        effective_text_root, active_text_root);

    EmitImportTransactionProgress(
        request.progress_observer, "replace_managed_text", 8U, kPhaseCount,
        active_text_root.string(), 0U, imported_payloads.size(), input_path,
        active_text_root, effective_text_root, active_text_root);
    text_root_updated = true;
    WriteImportedPayloadsToActiveTextRoot(active_text_root, imported_payloads);
    EmitImportTransactionProgress(
        request.progress_observer, "replace_managed_text", 8U, kPhaseCount,
        active_text_root.string(), imported_payloads.size(),
        imported_payloads.size(), input_path, active_text_root,
        effective_text_root, active_text_root);

    EmitImportTransactionProgress(
        request.progress_observer, "rebuild_database", 9U, kPhaseCount,
        effective_text_root.string(), 0U, 1U, input_path, active_text_root,
        effective_text_root, active_text_root);
    workflow_handler_.RunIngestReplacingAll(effective_text_root.string(),
                                            request.date_check_mode, false);
    EmitImportTransactionProgress(
        request.progress_observer, "rebuild_database", 9U, kPhaseCount,
        effective_text_root.string(), 1U, 1U, input_path, active_text_root,
        effective_text_root, active_text_root);

    EmitImportTransactionProgress(
        request.progress_observer, "cleanup", 10U, kPhaseCount,
        transaction_root.string(), 0U, 1U, input_path, active_text_root,
        transaction_root, runtime_work_root);
    std::error_code cleanup_error;
    fs::remove_all(transaction_root, cleanup_error);

    app_dto::TracerExchangeImportResult result{};
    result.ok = true;
    result.source_root_name = package.manifest.source_root_name;
    result.payload_file_count =
        static_cast<std::uint64_t>(imported_payloads.size());
    const std::uint64_t overlap_count = static_cast<std::uint64_t>(
        std::count_if(imported_payloads.begin(), imported_payloads.end(),
                      [&](const ImportedPayloadFile& imported_payload) {
                        return managed_month_files.contains(
                            imported_payload.month_key);
                      }));
    result.replaced_month_count =
        static_cast<std::uint64_t>(imported_payloads.size());
    result.preserved_month_count =
        static_cast<std::uint64_t>(managed_month_files.size()) - overlap_count;
    result.rebuilt_month_count =
        result.replaced_month_count + result.preserved_month_count;
    result.text_root_updated = true;
    result.config_applied = true;
    result.database_rebuilt = true;
    if (cleanup_error) {
      result.backup_retained_root = transaction_root;
      result.backup_cleanup_error = cleanup_error.message();
    }
    result.error_message = "";
    return result;
  } catch (const std::exception& error) {
    failure_message = error.what();
    try {
      const ActiveConverterConfigPaths active_paths =
          ResolveActiveConverterConfigPaths(
              request.active_converter_main_config_path);
      if (text_root_updated) {
        RestoreManagedTextFiles(active_text_root, backup_text_root,
                                added_month_keys);
      }
      if (config_applied) {
        RestoreBackupConfig(backup_config_root, active_paths);
      }
    } catch (const std::exception& rollback_error) {
      rollback_error_message = rollback_error.what();
    }
  } catch (...) {
    failure_message = "unexpected tracer exchange import failure";
    try {
      const ActiveConverterConfigPaths active_paths =
          ResolveActiveConverterConfigPaths(
              request.active_converter_main_config_path);
      if (text_root_updated) {
        RestoreManagedTextFiles(active_text_root, backup_text_root,
                                added_month_keys);
      }
      if (config_applied) {
        RestoreBackupConfig(backup_config_root, active_paths);
      }
    } catch (const std::exception& rollback_error) {
      rollback_error_message = rollback_error.what();
    }
  }

  app_dto::TracerExchangeImportResult failure_result{};
  failure_result.ok = false;
  failure_result.text_root_updated = false;
  failure_result.config_applied = false;
  failure_result.database_rebuilt = false;
  failure_result.retained_failure_root = transaction_root;
  failure_result.error_message =
      rollback_error_message.empty()
          ? failure_message
          : failure_message + " | rollback_failed: " + rollback_error_message;
  return failure_result;
}

}  // namespace tracer_core::infrastructure::crypto::tracer_exchange_internal
