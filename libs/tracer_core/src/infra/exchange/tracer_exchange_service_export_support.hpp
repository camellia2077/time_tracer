#ifndef INFRASTRUCTURE_CRYPTO_TRACER_EXCHANGE_SERVICE_EXPORT_SUPPORT_HPP_
#define INFRASTRUCTURE_CRYPTO_TRACER_EXCHANGE_SERVICE_EXPORT_SUPPORT_HPP_

#include <string>
#include <vector>

#include "infra/exchange/tracer_exchange_service_internal.hpp"

namespace tracer_core::infrastructure::crypto::tracer_exchange_internal {

struct ResolveEncryptOutputPathRequest {
  fs::path input_path;
  fs::path output_arg;
};

auto ResolveEncryptOutputPath(const ResolveEncryptOutputPathRequest& request)
    -> fs::path;
auto CurrentUtcTimestampRfc3339() -> std::string;

auto CollectInputPayloadFilesFromRoot(const fs::path& input_root)
    -> std::vector<InputPayloadFile>;
auto CollectInputPayloadFilesFromPayloads(
    const std::vector<app_dto::TracerExchangeTextPayloadItem>& payload_items)
    -> std::vector<InputPayloadFile>;

auto ValidateInputForExport(
    app_workflow::IWorkflowHandler& workflow_handler,
    tracer::core::domain::types::DateCheckMode date_check_mode,
    const fs::path& input_path) -> void;
auto ValidateInputPayloadsForExport(
    app_workflow::IWorkflowHandler& workflow_handler,
    tracer::core::domain::types::DateCheckMode date_check_mode,
    const std::vector<InputPayloadFile>& payload_files) -> void;

}  // namespace tracer_core::infrastructure::crypto::tracer_exchange_internal

#endif  // INFRASTRUCTURE_CRYPTO_TRACER_EXCHANGE_SERVICE_EXPORT_SUPPORT_HPP_
