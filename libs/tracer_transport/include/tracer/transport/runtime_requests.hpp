#pragma once

#include <optional>
#include <string>
#include <vector>

namespace tracer::transport {

struct IngestRequestPayload {
  std::string input_path;
  std::optional<std::string> date_check_mode;
  std::optional<bool> save_processed_output;
  std::optional<std::string> ingest_mode;
};

struct IngestSyncStatusRequestPayload {
  std::optional<std::vector<std::string>> months;
};

struct ConvertRequestPayload {
  std::string input_path;
  std::optional<std::string> date_check_mode;
  std::optional<bool> save_processed_output;
  std::optional<bool> validate_logic;
  std::optional<bool> validate_structure;
};

struct ImportRequestPayload {
  std::string processed_path;
};

struct ValidateStructureRequestPayload {
  std::string input_path;
};

struct ValidateLogicRequestPayload {
  std::string input_path;
  std::optional<std::string> date_check_mode;
};

struct RecordActivityAtomicallyRequestPayload {
  std::string target_date_iso;
  std::string raw_activity_name;
  std::string remark;
  std::optional<std::string> preferred_txt_path;
  std::optional<std::string> date_check_mode;
  std::optional<std::string> time_order_mode;
};

struct QueryRequestPayload {
  std::string action;
  std::optional<std::string> output_mode;
  std::optional<int> year;
  std::optional<int> month;
  std::optional<std::string> from_date;
  std::optional<std::string> to_date;
  std::optional<std::string> remark;
  std::optional<std::string> day_remark;
  std::optional<std::string> project;
  std::optional<std::string> root;
  std::optional<int> exercise;
  std::optional<int> status;
  std::optional<bool> overnight;
  std::optional<bool> reverse;
  std::optional<int> limit;
  std::optional<int> top_n;
  std::optional<int> lookback_days;
  std::optional<std::string> activity_prefix;
  std::optional<bool> activity_score_by_duration;
  std::optional<std::string> tree_period;
  std::optional<std::string> tree_period_argument;
  std::optional<int> tree_max_depth;
};

struct TemporalReportRequestPayload {
  std::string operation_kind;
  std::string display_mode;
  std::optional<std::string> selection_kind;
  std::optional<std::string> date;
  std::optional<std::string> start_date;
  std::optional<std::string> end_date;
  std::optional<int> days;
  std::optional<std::string> anchor_date;
  std::optional<std::string> format;
  std::optional<std::string> export_scope;
  std::optional<std::vector<int>> recent_days_list;
};

struct ReportBatchRequestPayload {
  std::vector<int> days_list;
  std::optional<std::string> format;
};

struct TreeRequestPayload {
  std::optional<bool> list_roots;
  std::optional<std::string> root_pattern;
  std::optional<int> max_depth;
  std::optional<std::string> period;
  std::optional<std::string> period_argument;
  std::optional<std::string> root;
};

}  // namespace tracer::transport
