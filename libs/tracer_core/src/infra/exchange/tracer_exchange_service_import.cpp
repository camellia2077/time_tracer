#include "infra/exchange/tracer_exchange_service_internal.hpp"

#include <algorithm>
#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

import tracer.core.infrastructure.exchange;

namespace tracer_core::infrastructure::crypto::tracer_exchange_internal {

namespace {

#include "infra/exchange/detail/tracer_exchange_service_import_payloads_impl.inc"

// Backup, rollback, effective-root assembly, and managed text updates.
#include "infra/exchange/detail/tracer_exchange_service_import_text_root_impl.inc"

// Transaction path layout, progress emission, rollback orchestration, and
// result assembly.
#include "infra/exchange/detail/tracer_exchange_service_import_support_impl.inc"

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
    throw std::invalid_argument(
        "Decrypt input path must be an existing file: " + input_path.string());
  }
  if (!HasExtensionCaseInsensitive(input_path, ".tracer")) {
    throw std::invalid_argument("Decrypt input file must be .tracer: " +
                                input_path.string());
  }

  const ImportTransactionPaths transaction_paths =
      BuildImportTransactionPaths(runtime_work_root, input_path);
  bool config_applied = false;
  bool text_root_updated = false;
  std::vector<ImportedPayloadFile> imported_payloads;
  std::string rollback_error_message;
  std::string failure_message;

  EnsureTransactionRootCreated(transaction_paths.transaction_root);

  try {
    constexpr std::size_t kPhaseCount = 10U;
    constexpr std::size_t kRebuildDatabasePhase = 9U;
    constexpr std::size_t kCleanupPhase = 10U;
    EmitImportTransactionProgress(
        request.progress_observer, "decrypt_package", 1U, kPhaseCount,
        input_path.filename().string(), 0U, 1U, input_path, active_text_root,
        input_path, transaction_paths.extracted_root);
    const file_crypto::FileCryptoPathContext kPathContext{
        .input_root_path = input_path.parent_path(),
        .output_root_path = transaction_paths.transaction_root,
        .current_input_path = input_path,
        .current_output_path = transaction_paths.transaction_root /
                               "exchange.ttpkg",
    };
    auto [decrypt_result, package_bytes] = file_crypto::DecryptFileToBytes(
        input_path, request.passphrase, kPathContext,
        BuildCryptoOptions(app_dto::TracerExchangeSecurityLevel::kInteractive,
                           {}));
    EnsureCryptoResultOk(decrypt_result, "Decrypt", input_path);
    EmitImportTransactionProgress(
        request.progress_observer, "decrypt_package", 1U, kPhaseCount,
        input_path.filename().string(), 1U, 1U, input_path, active_text_root,
        input_path, transaction_paths.extracted_root);

    EmitImportTransactionProgress(
        request.progress_observer, "validate_package_contract", 2U, kPhaseCount,
        input_path.filename().string(), 0U, 1U, input_path, active_text_root,
        input_path, transaction_paths.extracted_root);
    const exchange_pkg::DecodedTracerExchangePackage package =
        DecodePackageBytes(package_bytes);
    EmitImportTransactionProgress(
        request.progress_observer, "validate_package_contract", 2U, kPhaseCount,
        input_path.filename().string(), 1U, 1U, input_path, active_text_root,
        input_path, transaction_paths.extracted_root);

    WriteDecodedPackageToRoot(package, transaction_paths.extracted_root);
    EmitImportTransactionProgress(
        request.progress_observer, "validate_converter_config", 3U, kPhaseCount,
        "converter_config", 0U, 1U, input_path, active_text_root,
        transaction_paths.extracted_root /
            fs::path(exchange_pkg::kConverterMainPath),
        request.active_converter_main_config_path);
    ValidatePackageConverterConfig(transaction_paths.extracted_root);
    EmitImportTransactionProgress(
        request.progress_observer, "validate_converter_config", 3U, kPhaseCount,
        "converter_config", 1U, 1U, input_path, active_text_root,
        transaction_paths.extracted_root /
            fs::path(exchange_pkg::kConverterMainPath),
        request.active_converter_main_config_path);

    imported_payloads =
        CollectImportedPayloadFiles(transaction_paths.extracted_root);
    if (imported_payloads.empty()) {
      throw std::runtime_error(
          "Tracer exchange package must contain at least one canonical payload "
          "TXT file.");
    }
    const std::map<std::string, fs::path> managed_month_files =
        CollectManagedMonthFiles(active_text_root);

    const std::size_t effective_text_entry_count =
        managed_month_files.size() + imported_payloads.size();
    EmitImportTransactionProgress(
        request.progress_observer, "build_effective_text_view", 4U, kPhaseCount,
        "effective_text_root", 0U, effective_text_entry_count, input_path,
        active_text_root, active_text_root,
        transaction_paths.effective_text_root);
    BuildEffectiveTextRoot(managed_month_files, imported_payloads,
                           transaction_paths.effective_text_root);
    EmitImportTransactionProgress(
        request.progress_observer, "build_effective_text_view", 4U, kPhaseCount,
        "effective_text_root", effective_text_entry_count,
        effective_text_entry_count, input_path, active_text_root,
        active_text_root, transaction_paths.effective_text_root);

    const ActiveConverterConfigPaths active_paths =
        ResolveActiveConverterConfigPaths(
            request.active_converter_main_config_path);
    BackupActiveConverterConfig(active_paths,
                                transaction_paths.backup_config_root);
    BackupManagedTextFiles(active_text_root, imported_payloads,
                           managed_month_files,
                           transaction_paths.backup_text_root);

    EmitImportTransactionProgress(
        request.progress_observer, "apply_converter_config", 5U, kPhaseCount,
        "converter_config", 0U, 1U, input_path, active_text_root,
        transaction_paths.extracted_root /
            fs::path(exchange_pkg::kConverterMainPath),
        request.active_converter_main_config_path);
    config_applied = true;
    workflow_handler_.InstallActiveConverterConfig(
        (transaction_paths.extracted_root /
         fs::path(exchange_pkg::kConverterMainPath))
            .string(),
        request.active_converter_main_config_path.string());
    EmitImportTransactionProgress(
        request.progress_observer, "apply_converter_config", 5U, kPhaseCount,
        "converter_config", 1U, 1U, input_path, active_text_root,
        transaction_paths.extracted_root /
            fs::path(exchange_pkg::kConverterMainPath),
        request.active_converter_main_config_path);

    EmitImportTransactionProgress(
        request.progress_observer, "validate_text_structure", 6U, kPhaseCount,
        transaction_paths.effective_text_root.string(), 0U, 1U, input_path,
        active_text_root, transaction_paths.effective_text_root,
        active_text_root);
    workflow_handler_.RunValidateStructure(
        transaction_paths.effective_text_root.string());
    EmitImportTransactionProgress(
        request.progress_observer, "validate_text_structure", 6U, kPhaseCount,
        transaction_paths.effective_text_root.string(), 1U, 1U, input_path,
        active_text_root, transaction_paths.effective_text_root,
        active_text_root);

    EmitImportTransactionProgress(
        request.progress_observer, "validate_text_logic", 7U, kPhaseCount,
        transaction_paths.effective_text_root.string(), 0U, 1U, input_path,
        active_text_root, transaction_paths.effective_text_root,
        active_text_root);
    workflow_handler_.RunValidateLogic(
        transaction_paths.effective_text_root.string(),
        request.date_check_mode);
    EmitImportTransactionProgress(
        request.progress_observer, "validate_text_logic", 7U, kPhaseCount,
        transaction_paths.effective_text_root.string(), 1U, 1U, input_path,
        active_text_root, transaction_paths.effective_text_root,
        active_text_root);

    EmitImportTransactionProgress(
        request.progress_observer, "replace_managed_text", 8U, kPhaseCount,
        active_text_root.string(), 0U, imported_payloads.size(), input_path,
        active_text_root, transaction_paths.effective_text_root,
        active_text_root);
    text_root_updated = true;
    WriteImportedPayloadsToActiveTextRoot(active_text_root,
                                          managed_month_files,
                                          imported_payloads);
    EmitImportTransactionProgress(
        request.progress_observer, "replace_managed_text", 8U, kPhaseCount,
        active_text_root.string(), imported_payloads.size(),
        imported_payloads.size(), input_path, active_text_root,
        transaction_paths.effective_text_root, active_text_root);

    EmitImportTransactionProgress(
        request.progress_observer, "rebuild_database", kRebuildDatabasePhase,
        kPhaseCount,
        transaction_paths.effective_text_root.string(), 0U, 1U, input_path,
        active_text_root, transaction_paths.effective_text_root,
        active_text_root);
    workflow_handler_.RunIngestReplacingAll(
        transaction_paths.effective_text_root.string(), request.date_check_mode,
        false);
    EmitImportTransactionProgress(
        request.progress_observer, "rebuild_database", kRebuildDatabasePhase,
        kPhaseCount,
        transaction_paths.effective_text_root.string(), 1U, 1U, input_path,
        active_text_root, transaction_paths.effective_text_root,
        active_text_root);

    EmitImportTransactionProgress(
        request.progress_observer, "cleanup", kCleanupPhase, kPhaseCount,
        transaction_paths.transaction_root.string(), 0U, 1U, input_path,
        active_text_root, transaction_paths.transaction_root,
        runtime_work_root);
    std::error_code cleanup_error;
    fs::remove_all(transaction_paths.transaction_root, cleanup_error);

    return BuildSuccessfulImportResult(
        package, imported_payloads, managed_month_files,
        transaction_paths.transaction_root, cleanup_error);
  } catch (const std::exception& error) {
    failure_message = error.what();
    rollback_error_message = TryRollbackImportTransaction(
        workflow_handler_, active_text_root, transaction_paths.backup_text_root,
        transaction_paths.backup_config_root,
        request.active_converter_main_config_path, imported_payloads,
        text_root_updated, config_applied);
  } catch (...) {
    failure_message = "unexpected tracer exchange import failure";
    rollback_error_message = TryRollbackImportTransaction(
        workflow_handler_, active_text_root, transaction_paths.backup_text_root,
        transaction_paths.backup_config_root,
        request.active_converter_main_config_path, imported_payloads,
        text_root_updated, config_applied);
  }

  return BuildFailedImportResult(transaction_paths.transaction_root,
                                 failure_message, rollback_error_message);
}

}  // namespace tracer_core::infrastructure::crypto::tracer_exchange_internal
