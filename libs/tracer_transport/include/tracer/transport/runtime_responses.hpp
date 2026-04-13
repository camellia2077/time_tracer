#pragma once

#include <optional>
#include <string>
#include <vector>

namespace tracer::transport {

struct ErrorContractPayload {
  std::string error_code;
  std::string error_category;
  std::vector<std::string> hints;
};

struct IngestResponsePayload {
  bool ok = false;
  std::string error_message;
  ErrorContractPayload error_contract;
};

struct IngestSyncStatusItemPayload {
  std::string month_key;
  std::string txt_relative_path;
  std::string txt_content_hash_sha256;
  long long ingested_at_unix_ms = 0;
};

struct IngestSyncStatusResponsePayload {
  bool ok = false;
  std::string error_message;
  std::vector<IngestSyncStatusItemPayload> items;
  ErrorContractPayload error_contract;
};

struct AckResponsePayload {
  bool ok = false;
  std::string error_message;
  ErrorContractPayload error_contract;
};

struct TextResponsePayload {
  bool ok = false;
  std::string error_message;
  std::string content;
  ErrorContractPayload error_contract;
};

struct RuntimeCheckResponsePayload {
  bool ok = false;
  std::string error_message;
  std::vector<std::string> messages;
  ErrorContractPayload error_contract;
};

struct CliGlobalDefaultsPayload {
  std::optional<std::string> default_format;
};

struct CliCommandDefaultsPayload {
  std::optional<std::string> export_format;
  std::optional<std::string> query_format;
  std::optional<std::string> convert_date_check_mode;
  std::optional<bool> convert_save_processed_output;
  std::optional<bool> convert_validate_logic;
  std::optional<bool> convert_validate_structure;
  std::optional<std::string> ingest_date_check_mode;
  std::optional<bool> ingest_save_processed_output;
  std::optional<std::string> validate_logic_date_check_mode;
};

struct CliConfigPayload {
  bool default_save_processed_output = false;
  std::optional<std::string> default_date_check_mode;
  CliGlobalDefaultsPayload defaults;
  CliCommandDefaultsPayload command_defaults;
};

struct ResolvedCliPathsPayload {
  std::string db_path;
  std::string runtime_output_root;
  std::string converter_config_toml_path;
  std::optional<std::string> exe_dir;
  std::optional<std::string> output_root;
  std::optional<std::string> export_root;
};

struct ResolveCliContextResponsePayload {
  bool ok = false;
  std::string error_message;
  std::optional<ResolvedCliPathsPayload> paths;
  std::optional<CliConfigPayload> cli_config;
  ErrorContractPayload error_contract;
};

struct CapabilitiesAbiPayload {
  std::string name;
  int version = 0;
};

struct CapabilitiesFeaturesPayload {
  bool build_info_json = false;
  bool command_contract_json = false;
  bool runtime_log_callback = false;
  bool runtime_diagnostics_callback = false;
  bool runtime_crypto_progress_callback = false;
  bool runtime_ingest_json = false;
  bool runtime_ingest_sync_status_json = false;
  bool runtime_convert_json = false;
  bool runtime_import_json = false;
  bool runtime_validate_structure_json = false;
  bool runtime_validate_logic_json = false;
  bool runtime_record_activity_atomically_json = false;
  bool runtime_txt_json = false;
  bool runtime_query_json = false;
  bool runtime_temporal_report_json = false;
  bool runtime_report_batch_json = false;
  bool runtime_tree_json = false;
  bool processed_json_io = false;
  bool report_markdown = false;
  bool report_latex = false;
  bool report_typst = false;
};

struct CapabilitiesResponsePayload {
  CapabilitiesAbiPayload abi;
  CapabilitiesFeaturesPayload features;
};

struct QueryResponsePayload {
  bool ok = false;
  std::string error_message;
  std::string content;
  ErrorContractPayload error_contract;
};

struct ReportResponsePayload {
  bool ok = false;
  std::string error_message;
  std::string content;
  ErrorContractPayload error_contract;
};

struct ReportBatchResponsePayload {
  bool ok = false;
  std::string error_message;
  std::string content;
  ErrorContractPayload error_contract;
};

struct ReportTargetsResponsePayload {
  bool ok = false;
  std::string error_message;
  std::string type;
  std::vector<std::string> items;
  ErrorContractPayload error_contract;
};

struct ExportResponsePayload {
  bool ok = false;
  std::string error_message;
  ErrorContractPayload error_contract;
};

struct ProjectTreeNodePayload {
  std::string name;
  std::optional<std::string> path;
  std::optional<long long> duration_seconds;
  std::vector<ProjectTreeNodePayload> children;
};

struct TreeResponsePayload {
  bool ok = false;
  bool found = true;
  std::string error_message;
  std::vector<std::string> roots;
  std::vector<ProjectTreeNodePayload> nodes;
  ErrorContractPayload error_contract;
};

}  // namespace tracer::transport
