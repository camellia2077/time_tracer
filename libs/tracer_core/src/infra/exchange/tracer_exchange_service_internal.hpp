#ifndef INFRASTRUCTURE_CRYPTO_TRACER_EXCHANGE_SERVICE_INTERNAL_HPP_
#define INFRASTRUCTURE_CRYPTO_TRACER_EXCHANGE_SERVICE_INTERNAL_HPP_

#include <filesystem>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "application/dto/exchange_requests.hpp"
#include "application/dto/exchange_responses.hpp"
#include "application/interfaces/i_workflow_handler.hpp"
#include "application/ports/exchange/i_tracer_exchange_service.hpp"
#include "infra/crypto/file_crypto_service.hpp"

namespace tracer::core::infrastructure::crypto::exchange {

struct DecodedTracerExchangePackage;

}  // namespace tracer::core::infrastructure::crypto::exchange

namespace tracer_core::infrastructure::crypto::tracer_exchange_internal {

namespace app_dto = tracer_core::core::dto;
namespace app_ports = tracer_core::application::ports;
namespace app_workflow = tracer::core::application::workflow;
namespace exchange_pkg = tracer::core::infrastructure::crypto::exchange;
namespace fs = std::filesystem;
namespace file_crypto = tracer_core::infrastructure::crypto;

struct InputPayloadFile {
  fs::path source_path;
  std::string source_label;
  std::vector<std::uint8_t> content_bytes;
  std::string relative_package_path;
  std::string month_key;
  int year = 0;
  int month = 0;
};

struct ParsedMonthInfo {
  int year = 0;
  int month = 0;
  std::string month_key;
  std::string file_name;
};

struct ImportedPayloadFile {
  fs::path source_path;
  std::string relative_package_path;
  std::string month_key;
  int year = 0;
  int month = 0;
};

struct ActiveConverterConfigPaths {
  fs::path main_config_path;
  fs::path alias_mapping_path;
  fs::path duration_rules_path;
};

class TracerExchangeService final : public app_ports::ITracerExchangeService {
 public:
  explicit TracerExchangeService(
      app_workflow::IWorkflowHandler& workflow_handler)
      : workflow_handler_(workflow_handler) {}

  auto RunExport(const app_dto::TracerExchangeExportRequest& request)
      -> app_dto::TracerExchangeExportResult override;
  auto RunImport(const app_dto::TracerExchangeImportRequest& request)
      -> app_dto::TracerExchangeImportResult override;
  auto RunInspect(const app_dto::TracerExchangeInspectRequest& request)
      -> app_dto::TracerExchangeInspectResult override;

 private:
  app_workflow::IWorkflowHandler& workflow_handler_;
};

auto ToLowerAscii(std::string value) -> std::string;
auto HasExtensionCaseInsensitive(const fs::path& path,
                                 std::string_view ext_lower) -> bool;
auto ParseMonthInfoFromFileName(std::string_view file_name,
                                std::string_view source_label)
    -> ParsedMonthInfo;
auto ParseMonthInfoFromCanonicalText(std::span<const std::uint8_t> bytes,
                                     std::string_view source_label)
    -> ParsedMonthInfo;
auto SanitizeStem(std::string value) -> std::string;
auto BuildUniqueSuffix() -> std::string;
auto BuildScopedStagingDir(const fs::path& writable_root,
                           std::string_view purpose, std::string_view stem)
    -> fs::path;
auto EnsureDirectoryRemoved(const fs::path& path) -> void;
auto RemoveDirectoryBestEffort(const fs::path& path) -> void;
auto EnsureParentDirectory(const fs::path& path) -> void;
auto ReadFileBytes(const fs::path& path) -> std::vector<std::uint8_t>;
auto WriteFileBytes(const fs::path& path, std::span<const std::uint8_t> bytes)
    -> void;
auto IsCanonicalTextPackagePath(std::string_view relative_path) -> bool;
auto CanonicalizePackageTextBytes(std::span<const std::uint8_t> bytes,
                                  std::string_view source_label)
    -> std::vector<std::uint8_t>;
auto CanonicalizePackageTextBytes(std::string_view text,
                                  std::string_view source_label)
    -> std::vector<std::uint8_t>;
auto EnsureRegularFileExists(const fs::path& path, std::string_view label)
    -> void;
auto EnsureCryptoResultOk(const file_crypto::FileCryptoResult& result,
                          std::string_view action, const fs::path& input_path)
    -> void;
auto ResolveActiveConverterConfigPaths(
    const fs::path& active_converter_main_config_path)
    -> ActiveConverterConfigPaths;
auto EnsureActiveConverterConfigExists(
    const ActiveConverterConfigPaths& active_paths) -> void;
auto BackupActiveConverterConfig(const ActiveConverterConfigPaths& active_paths,
                                 const fs::path& backup_root) -> void;
auto ValidatePackageConverterConfig(const fs::path& work_root) -> void;
auto BuildCryptoOptions(
    app_dto::TracerExchangeSecurityLevel security_level,
    const app_dto::TracerExchangeProgressObserver& progress_observer)
    -> file_crypto::FileCryptoOptions;
auto WriteDecodedPackageToRoot(
    const exchange_pkg::DecodedTracerExchangePackage& package,
    const fs::path& root) -> void;

}  // namespace tracer_core::infrastructure::crypto::tracer_exchange_internal

#endif  // INFRASTRUCTURE_CRYPTO_TRACER_EXCHANGE_SERVICE_INTERNAL_HPP_
