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

  const fs::path kInputPath = fs::absolute(request.input_tracer_path);
  const fs::path kActiveTextRoot = fs::absolute(request.active_text_root_path);
  const fs::path kRuntimeWorkRoot = fs::absolute(request.runtime_work_root);
  if (!fs::exists(kInputPath) || !fs::is_regular_file(kInputPath)) {
    throw std::invalid_argument(
        "Decrypt input path must be an existing file: " + kInputPath.string());
  }
  if (!HasExtensionCaseInsensitive(kInputPath, ".tracer")) {
    throw std::invalid_argument("Decrypt input file must be .tracer: " +
                                kInputPath.string());
  }

  const ImportTransactionPaths kTransactionPaths =
      BuildImportTransactionPaths(kRuntimeWorkRoot, kInputPath);
  bool config_applied = false;
  bool text_root_updated = false;
  std::vector<ImportedPayloadFile> imported_payloads;
  std::string rollback_error_message;
  std::string failure_message;

  EnsureTransactionRootCreated(kTransactionPaths.transaction_root);

  try {
    constexpr std::size_t kPhaseCount = 10U;
    constexpr std::size_t kRebuildDatabasePhase = 9U;
    constexpr std::size_t kCleanupPhase = 10U;
    EmitImportTransactionProgress(
        request.progress_observer, "decrypt_package", 1U, kPhaseCount,
        kInputPath.filename().string(), 0U, 1U, kInputPath, kActiveTextRoot,
        kInputPath, kTransactionPaths.extracted_root);
    const file_crypto::FileCryptoPathContext kPathContext{
        .input_root_path = kInputPath.parent_path(),
        .output_root_path = kTransactionPaths.transaction_root,
        .current_input_path = kInputPath,
        .current_output_path = kTransactionPaths.transaction_root /
                               "exchange.ttpkg",
    };
    auto [decrypt_result, package_bytes] = file_crypto::DecryptFileToBytes(
        kInputPath, request.passphrase, kPathContext,
        BuildCryptoOptions(app_dto::TracerExchangeSecurityLevel::kInteractive,
                           {}));
    EnsureCryptoResultOk(decrypt_result, "Decrypt", kInputPath);
    EmitImportTransactionProgress(
        request.progress_observer, "decrypt_package", 1U, kPhaseCount,
        kInputPath.filename().string(), 1U, 1U, kInputPath, kActiveTextRoot,
        kInputPath, kTransactionPaths.extracted_root);

    EmitImportTransactionProgress(
        request.progress_observer, "validate_package_contract", 2U, kPhaseCount,
        kInputPath.filename().string(), 0U, 1U, kInputPath, kActiveTextRoot,
        kInputPath, kTransactionPaths.extracted_root);
    const exchange_pkg::DecodedTracerExchangePackage kPackage =
        DecodePackageBytes(package_bytes);
    EmitImportTransactionProgress(
        request.progress_observer, "validate_package_contract", 2U, kPhaseCount,
        kInputPath.filename().string(), 1U, 1U, kInputPath, kActiveTextRoot,
        kInputPath, kTransactionPaths.extracted_root);

    WriteDecodedPackageToRoot(kPackage, kTransactionPaths.extracted_root);
    EmitImportTransactionProgress(
        request.progress_observer, "validate_converter_config", 3U, kPhaseCount,
        "converter_config", 0U, 1U, kInputPath, kActiveTextRoot,
        kTransactionPaths.extracted_root /
            fs::path(exchange_pkg::kConverterMainPath),
        request.active_converter_main_config_path);
    ValidatePackageConverterConfig(kTransactionPaths.extracted_root);
    EmitImportTransactionProgress(
        request.progress_observer, "validate_converter_config", 3U, kPhaseCount,
        "converter_config", 1U, 1U, kInputPath, kActiveTextRoot,
        kTransactionPaths.extracted_root /
            fs::path(exchange_pkg::kConverterMainPath),
        request.active_converter_main_config_path);

    imported_payloads =
        CollectImportedPayloadFiles(kTransactionPaths.extracted_root);
    if (imported_payloads.empty()) {
      throw std::runtime_error(
          "Tracer exchange package must contain at least one canonical payload "
          "TXT file.");
    }
    const std::map<std::string, fs::path> kManagedMonthFiles =
        CollectManagedMonthFiles(kActiveTextRoot);

    const std::size_t kEffectiveTextEntryCount =
        kManagedMonthFiles.size() + imported_payloads.size();
    EmitImportTransactionProgress(
        request.progress_observer, "build_effective_text_view", 4U, kPhaseCount,
        "effective_text_root", 0U, kEffectiveTextEntryCount, kInputPath,
        kActiveTextRoot, kActiveTextRoot,
        kTransactionPaths.effective_text_root);
    BuildEffectiveTextRoot(kManagedMonthFiles, imported_payloads,
                           kTransactionPaths.effective_text_root);
    EmitImportTransactionProgress(
        request.progress_observer, "build_effective_text_view", 4U, kPhaseCount,
        "effective_text_root", kEffectiveTextEntryCount,
        kEffectiveTextEntryCount, kInputPath, kActiveTextRoot,
        kActiveTextRoot, kTransactionPaths.effective_text_root);

    const ActiveConverterConfigPaths kActivePaths =
        ResolveActiveConverterConfigPaths(
            request.active_converter_main_config_path);
    BackupActiveConverterConfig(kActivePaths,
                                kTransactionPaths.backup_config_root);
    BackupManagedTextFiles(kActiveTextRoot, imported_payloads,
                           kManagedMonthFiles,
                           kTransactionPaths.backup_text_root);

    EmitImportTransactionProgress(
        request.progress_observer, "apply_converter_config", 5U, kPhaseCount,
        "converter_config", 0U, 1U, kInputPath, kActiveTextRoot,
        kTransactionPaths.extracted_root /
            fs::path(exchange_pkg::kConverterMainPath),
        request.active_converter_main_config_path);
    config_applied = true;
    workflow_handler_.InstallActiveConverterConfig({
        .source_main_config_path =
            (kTransactionPaths.extracted_root /
             fs::path(exchange_pkg::kConverterMainPath))
                .string(),
        .target_main_config_path =
            request.active_converter_main_config_path.string(),
    });
    EmitImportTransactionProgress(
        request.progress_observer, "apply_converter_config", 5U, kPhaseCount,
        "converter_config", 1U, 1U, kInputPath, kActiveTextRoot,
        kTransactionPaths.extracted_root /
            fs::path(exchange_pkg::kConverterMainPath),
        request.active_converter_main_config_path);

    EmitImportTransactionProgress(
        request.progress_observer, "validate_text_structure", 6U, kPhaseCount,
        kTransactionPaths.effective_text_root.string(), 0U, 1U, kInputPath,
        kActiveTextRoot, kTransactionPaths.effective_text_root,
        kActiveTextRoot);
    workflow_handler_.RunValidateStructure(
        kTransactionPaths.effective_text_root.string());
    EmitImportTransactionProgress(
        request.progress_observer, "validate_text_structure", 6U, kPhaseCount,
        kTransactionPaths.effective_text_root.string(), 1U, 1U, kInputPath,
        kActiveTextRoot, kTransactionPaths.effective_text_root,
        kActiveTextRoot);

    EmitImportTransactionProgress(
        request.progress_observer, "validate_text_logic", 7U, kPhaseCount,
        kTransactionPaths.effective_text_root.string(), 0U, 1U, kInputPath,
        kActiveTextRoot, kTransactionPaths.effective_text_root,
        kActiveTextRoot);
    workflow_handler_.RunValidateLogic(
        kTransactionPaths.effective_text_root.string(),
        request.date_check_mode);
    EmitImportTransactionProgress(
        request.progress_observer, "validate_text_logic", 7U, kPhaseCount,
        kTransactionPaths.effective_text_root.string(), 1U, 1U, kInputPath,
        kActiveTextRoot, kTransactionPaths.effective_text_root,
        kActiveTextRoot);

    EmitImportTransactionProgress(
        request.progress_observer, "replace_managed_text", 8U, kPhaseCount,
        kActiveTextRoot.string(), 0U, imported_payloads.size(), kInputPath,
        kActiveTextRoot, kTransactionPaths.effective_text_root,
        kActiveTextRoot);
    text_root_updated = true;
    WriteImportedPayloadsToActiveTextRoot(kActiveTextRoot,
                                          kManagedMonthFiles,
                                          imported_payloads);
    EmitImportTransactionProgress(
        request.progress_observer, "replace_managed_text", 8U, kPhaseCount,
        kActiveTextRoot.string(), imported_payloads.size(),
        imported_payloads.size(), kInputPath, kActiveTextRoot,
        kTransactionPaths.effective_text_root, kActiveTextRoot);

    EmitImportTransactionProgress(
        request.progress_observer, "rebuild_database", kRebuildDatabasePhase,
        kPhaseCount,
        kTransactionPaths.effective_text_root.string(), 0U, 1U, kInputPath,
        kActiveTextRoot, kTransactionPaths.effective_text_root,
        kActiveTextRoot);
    workflow_handler_.RunIngestReplacingAll(
        kTransactionPaths.effective_text_root.string(), request.date_check_mode,
        false);
    EmitImportTransactionProgress(
        request.progress_observer, "rebuild_database", kRebuildDatabasePhase,
        kPhaseCount,
        kTransactionPaths.effective_text_root.string(), 1U, 1U, kInputPath,
        kActiveTextRoot, kTransactionPaths.effective_text_root,
        kActiveTextRoot);

    EmitImportTransactionProgress(
        request.progress_observer, "cleanup", kCleanupPhase, kPhaseCount,
        kTransactionPaths.transaction_root.string(), 0U, 1U, kInputPath,
        kActiveTextRoot, kTransactionPaths.transaction_root,
        kRuntimeWorkRoot);
    std::error_code cleanup_error;
    fs::remove_all(kTransactionPaths.transaction_root, cleanup_error);

    return BuildSuccessfulImportResult(
        kPackage, imported_payloads, kManagedMonthFiles,
        kTransactionPaths.transaction_root, cleanup_error);
  } catch (const std::exception& error) {
    failure_message = error.what();
    rollback_error_message = TryRollbackImportTransaction(
        workflow_handler_, kActiveTextRoot, kTransactionPaths.backup_text_root,
        kTransactionPaths.backup_config_root,
        request.active_converter_main_config_path, imported_payloads,
        text_root_updated, config_applied);
  } catch (...) {
    failure_message = "unexpected tracer exchange import failure";
    rollback_error_message = TryRollbackImportTransaction(
        workflow_handler_, kActiveTextRoot, kTransactionPaths.backup_text_root,
        kTransactionPaths.backup_config_root,
        request.active_converter_main_config_path, imported_payloads,
        text_root_updated, config_applied);
  }

  return BuildFailedImportResult(kTransactionPaths.transaction_root,
                                 failure_message, rollback_error_message);
}

}  // namespace tracer_core::infrastructure::crypto::tracer_exchange_internal
