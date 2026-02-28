// bootstrap/cli_runtime_factory_proxy.cpp
#include "bootstrap/cli_runtime_factory_proxy.hpp"

#include <array>
#include <exception>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "domain/reports/types/report_types.hpp"
#include "domain/types/date_check_mode.hpp"
#include "shared/types/exceptions.hpp"
#include "tracer/transport/envelope.hpp"
#include "tracer/transport/runtime_codec.hpp"

namespace tracer_core::cli::bootstrap::internal {

namespace {

using tracer_core::common::LogicError;
using tracer_core::core::dto::DataQueryAction;
using tracer_core::core::dto::DataQueryRequest;
using tracer_core::core::dto::OperationAck;
using tracer_core::core::dto::ReportExportRequest;
using tracer_core::core::dto::ReportExportType;
using tracer_core::core::dto::ReportQueryRequest;
using tracer_core::core::dto::ReportQueryType;
using tracer_core::core::dto::TextOutput;
using tracer_core::core::dto::TreeQueryRequest;
using tracer_core::core::dto::TreeQueryResponse;
namespace tt_transport = tracer::transport;

template <typename TEnum, std::size_t N>
[[nodiscard]] auto
EnumTokenOr(TEnum value,
            const std::array<std::pair<TEnum, std::string_view>, N> &mapping,
            std::string_view fallback) -> std::string_view {
  for (const auto &[item, token] : mapping) {
    if (item == value) {
      return token;
    }
  }
  return fallback;
}

auto ConvertTreeNode(const tt_transport::ProjectTreeNodePayload &node_payload)
    -> ProjectTreeNode {
  ProjectTreeNode node{};
  node.name = node_payload.name;
  node.path = node_payload.path.value_or(std::string{});
  node.duration_seconds = node_payload.duration_seconds;
  node.children.reserve(node_payload.children.size());
  for (const auto &child : node_payload.children) {
    node.children.push_back(ConvertTreeNode(child));
  }
  return node;
}

[[nodiscard]] auto ToDateModeToken(DateCheckMode mode) -> std::string_view {
  static constexpr std::array kMapping = {
      std::pair{DateCheckMode::kNone, std::string_view{"none"}},
      std::pair{DateCheckMode::kContinuity, std::string_view{"continuity"}},
      std::pair{DateCheckMode::kFull, std::string_view{"full"}},
  };
  return EnumTokenOr(mode, kMapping, "none");
}

[[nodiscard]] auto ToIngestModeToken(IngestMode mode) -> std::string_view {
  static constexpr std::array kMapping = {
      std::pair{IngestMode::kStandard, std::string_view{"standard"}},
      std::pair{IngestMode::kSingleTxtReplaceMonth,
                std::string_view{"single_txt_replace_month"}},
  };
  return EnumTokenOr(mode, kMapping, "standard");
}

[[nodiscard]] auto ToFormatToken(ReportFormat format) -> std::string_view {
  static constexpr std::array kMapping = {
      std::pair{ReportFormat::kMarkdown, std::string_view{"md"}},
      std::pair{ReportFormat::kLaTeX, std::string_view{"tex"}},
      std::pair{ReportFormat::kTyp, std::string_view{"typ"}},
  };
  return EnumTokenOr(format, kMapping, "md");
}

[[nodiscard]] auto ToReportTypeToken(ReportQueryType type) -> std::string_view {
  static constexpr std::array kMapping = {
      std::pair{ReportQueryType::kDay, std::string_view{"day"}},
      std::pair{ReportQueryType::kMonth, std::string_view{"month"}},
      std::pair{ReportQueryType::kRecent, std::string_view{"recent"}},
      std::pair{ReportQueryType::kRange, std::string_view{"range"}},
      std::pair{ReportQueryType::kWeek, std::string_view{"week"}},
      std::pair{ReportQueryType::kYear, std::string_view{"year"}},
  };
  return EnumTokenOr(type, kMapping, "day");
}

[[nodiscard]] auto ToExportTypeToken(ReportExportType type)
    -> std::string_view {
  static constexpr std::array kMapping = {
      std::pair{ReportExportType::kDay, std::string_view{"day"}},
      std::pair{ReportExportType::kMonth, std::string_view{"month"}},
      std::pair{ReportExportType::kRecent, std::string_view{"recent"}},
      std::pair{ReportExportType::kWeek, std::string_view{"week"}},
      std::pair{ReportExportType::kYear, std::string_view{"year"}},
      std::pair{ReportExportType::kAllDay, std::string_view{"all-day"}},
      std::pair{ReportExportType::kAllMonth, std::string_view{"all-month"}},
      std::pair{ReportExportType::kAllRecent, std::string_view{"all-recent"}},
      std::pair{ReportExportType::kAllWeek, std::string_view{"all-week"}},
      std::pair{ReportExportType::kAllYear, std::string_view{"all-year"}},
  };
  return EnumTokenOr(type, kMapping, "day");
}

[[nodiscard]] auto ToDataActionToken(DataQueryAction action)
    -> std::string_view {
  static constexpr std::array kMapping = {
      std::pair{DataQueryAction::kYears, std::string_view{"years"}},
      std::pair{DataQueryAction::kMonths, std::string_view{"months"}},
      std::pair{DataQueryAction::kDays, std::string_view{"days"}},
      std::pair{DataQueryAction::kDaysDuration,
                std::string_view{"days_duration"}},
      std::pair{DataQueryAction::kDaysStats, std::string_view{"days_stats"}},
      std::pair{DataQueryAction::kSearch, std::string_view{"search"}},
      std::pair{DataQueryAction::kActivitySuggest,
                std::string_view{"activity_suggest"}},
      std::pair{DataQueryAction::kMappingNames,
                std::string_view{"mapping_names"}},
      std::pair{DataQueryAction::kReportChart,
                std::string_view{"report_chart"}},
      std::pair{DataQueryAction::kTree, std::string_view{"tree"}},
  };
  return EnumTokenOr(action, kMapping, "years");
}

[[nodiscard]] auto
ToDataOutputModeToken(tracer_core::core::dto::DataQueryOutputMode mode)
    -> std::string_view {
  using tracer_core::core::dto::DataQueryOutputMode;
  static constexpr std::array kMapping = {
      std::pair{DataQueryOutputMode::kText, std::string_view{"text"}},
      std::pair{DataQueryOutputMode::kSemanticJson,
                std::string_view{"semantic_json"}},
  };
  return EnumTokenOr(mode, kMapping, "text");
}

class CoreApiProxy final : public ITracerCoreApi {
public:
  CoreApiProxy(std::shared_ptr<CoreLibrary> library,
               TtCoreRuntimeHandle *runtime_handle)
      : library_(std::move(library)), runtime_handle_(runtime_handle) {}

  ~CoreApiProxy() override {
    if (library_ != nullptr && runtime_handle_ != nullptr) {
      library_->symbols().runtime_destroy(runtime_handle_);
      runtime_handle_ = nullptr;
    }
  }

  auto RunConvert(const tracer_core::core::dto::ConvertRequest &request)
      -> OperationAck override {
    tt_transport::ConvertRequestPayload payload{};
    payload.input_path = request.input_path;
    payload.date_check_mode =
        std::string(ToDateModeToken(request.date_check_mode));
    payload.save_processed_output = request.save_processed_output;
    payload.validate_logic = request.validate_logic;
    payload.validate_structure = request.validate_structure;
    return CallAck(library_->symbols().runtime_convert,
                   tt_transport::EncodeConvertRequest(payload),
                   "runtime_convert");
  }

  auto RunIngest(const tracer_core::core::dto::IngestRequest &request)
      -> OperationAck override {
    tt_transport::IngestRequestPayload payload{};
    payload.input_path = request.input_path;
    payload.date_check_mode =
        std::string(ToDateModeToken(request.date_check_mode));
    payload.save_processed_output = request.save_processed_output;
    payload.ingest_mode = std::string(ToIngestModeToken(request.ingest_mode));
    return CallAck(library_->symbols().runtime_ingest,
                   tt_transport::EncodeIngestRequest(payload),
                   "runtime_ingest");
  }

  auto RunImport(const tracer_core::core::dto::ImportRequest &request)
      -> OperationAck override {
    tt_transport::ImportRequestPayload payload{};
    payload.processed_path = request.processed_path;
    return CallAck(library_->symbols().runtime_import,
                   tt_transport::EncodeImportRequest(payload),
                   "runtime_import");
  }

  auto RunValidateStructure(
      const tracer_core::core::dto::ValidateStructureRequest &request)
      -> OperationAck override {
    tt_transport::ValidateStructureRequestPayload payload{};
    payload.input_path = request.input_path;
    return CallAck(library_->symbols().runtime_validate_structure,
                   tt_transport::EncodeValidateStructureRequest(payload),
                   "runtime_validate_structure");
  }

  auto
  RunValidateLogic(const tracer_core::core::dto::ValidateLogicRequest &request)
      -> OperationAck override {
    tt_transport::ValidateLogicRequestPayload payload{};
    payload.input_path = request.input_path;
    payload.date_check_mode =
        std::string(ToDateModeToken(request.date_check_mode));
    return CallAck(library_->symbols().runtime_validate_logic,
                   tt_transport::EncodeValidateLogicRequest(payload),
                   "runtime_validate_logic");
  }

  auto RunReportQuery(const ReportQueryRequest &request)
      -> TextOutput override {
    tt_transport::ReportRequestPayload payload{};
    payload.type = std::string(ToReportTypeToken(request.type));
    payload.argument = request.argument;
    payload.format = std::string(ToFormatToken(request.format));
    return CallText(library_->symbols().runtime_report,
                    tt_transport::EncodeReportRequest(payload),
                    "runtime_report");
  }

  auto RunStructuredReportQuery(
      const tracer_core::core::dto::StructuredReportQueryRequest & /*request*/)
      -> tracer_core::core::dto::StructuredReportOutput override {
    return {.ok = false,
            .error_message =
                "Structured report query is not exposed by C API yet."};
  }

  auto RunPeriodBatchQuery(
      const tracer_core::core::dto::PeriodBatchQueryRequest &request)
      -> TextOutput override {
    tt_transport::ReportBatchRequestPayload payload{};
    payload.days_list = request.days_list;
    payload.format = std::string(ToFormatToken(request.format));
    return CallText(library_->symbols().runtime_report_batch,
                    tt_transport::EncodeReportBatchRequest(payload),
                    "runtime_report_batch");
  }

  auto RunStructuredPeriodBatchQuery(
      const tracer_core::core::dto::StructuredPeriodBatchQueryRequest
          & /*request*/)
      -> tracer_core::core::dto::StructuredPeriodBatchOutput override {
    return {.ok = false,
            .items = {},
            .error_message =
                "Structured period batch query is not exposed by C API yet."};
  }

  auto RunReportExport(const ReportExportRequest &request)
      -> OperationAck override {
    tt_transport::ExportRequestPayload payload{};
    payload.type = std::string(ToExportTypeToken(request.type));
    payload.format = std::string(ToFormatToken(request.format));
    if (!request.argument.empty()) {
      payload.argument = request.argument;
    }
    if (!request.recent_days_list.empty()) {
      payload.recent_days_list = request.recent_days_list;
    }
    return CallAck(library_->symbols().runtime_export,
                   tt_transport::EncodeExportRequest(payload),
                   "runtime_export");
  }

  auto RunDataQuery(const DataQueryRequest &request) -> TextOutput override {
    tt_transport::QueryRequestPayload payload{};
    payload.action = std::string(ToDataActionToken(request.action));
    payload.output_mode =
        std::string(ToDataOutputModeToken(request.output_mode));
    payload.year = request.year;
    payload.month = request.month;
    payload.from_date = request.from_date;
    payload.to_date = request.to_date;
    payload.remark = request.remark;
    payload.day_remark = request.day_remark;
    payload.project = request.project;
    payload.root = request.root;
    payload.exercise = request.exercise;
    payload.status = request.status;
    payload.overnight = request.overnight;
    payload.reverse = request.reverse;
    payload.limit = request.limit;
    payload.top_n = request.top_n;
    payload.lookback_days = request.lookback_days;
    payload.activity_prefix = request.activity_prefix;
    payload.activity_score_by_duration = request.activity_score_by_duration;
    payload.tree_period = request.tree_period;
    payload.tree_period_argument = request.tree_period_argument;
    payload.tree_max_depth = request.tree_max_depth;
    return CallText(library_->symbols().runtime_query,
                    tt_transport::EncodeQueryRequest(payload), "runtime_query");
  }

  auto RunTreeQuery(const TreeQueryRequest &request)
      -> TreeQueryResponse override {
    tt_transport::TreeRequestPayload payload{};
    payload.list_roots = request.list_roots;
    payload.root_pattern = request.root_pattern;
    payload.max_depth = request.max_depth;
    return CallTree(library_->symbols().runtime_tree,
                    tt_transport::EncodeTreeRequest(payload), "runtime_tree");
  }

private:
  template <typename Fn>
  auto CallRawResponse(Fn function, std::string_view request_json) const
      -> std::string {
    const std::string request(request_json);
    const char *response = function(runtime_handle_, request.c_str());
    if (response == nullptr || response[0] == '\0') {
      throw LogicError("Core runtime returned empty response.");
    }
    return std::string(response);
  }

  template <typename Fn>
  auto CallAck(Fn function, std::string_view request_json,
               std::string_view context) const -> OperationAck {
    try {
      const auto decoded = tt_transport::DecodeAckResponse(
          CallRawResponse(function, request_json), context);
      return OperationAck{
          .ok = decoded.ok,
          .error_message = std::move(decoded.error_message),
      };
    } catch (const std::exception &error) {
      throw LogicError(error.what());
    }
  }

  template <typename Fn>
  auto CallText(Fn function, std::string_view request_json,
                std::string_view context) const -> TextOutput {
    try {
      const auto decoded = tt_transport::DecodeTextResponse(
          CallRawResponse(function, request_json), context);
      return TextOutput{
          .ok = decoded.ok,
          .content = std::move(decoded.content),
          .error_message = std::move(decoded.error_message),
      };
    } catch (const std::exception &error) {
      throw LogicError(error.what());
    }
  }

  template <typename Fn>
  auto CallTree(Fn function, std::string_view request_json,
                std::string_view context) const -> TreeQueryResponse {
    const std::string response_raw = CallRawResponse(function, request_json);

    tt_transport::TreeResponsePayload tree_payload{};
    try {
      tree_payload = tt_transport::DecodeTreeResponse(response_raw);
    } catch (const std::exception &error) {
      throw LogicError(std::string(context) +
                       ": invalid JSON response payload: " + error.what());
    }

    TreeQueryResponse out{};
    out.ok = tree_payload.ok;
    out.error_message = std::move(tree_payload.error_message);
    if (!out.ok) {
      if (out.error_message.empty()) {
        out.error_message = "Core operation failed.";
      }
      return out;
    }

    out.found = tree_payload.found;
    out.roots = std::move(tree_payload.roots);
    out.nodes.reserve(tree_payload.nodes.size());
    for (const auto &node_payload : tree_payload.nodes) {
      out.nodes.push_back(ConvertTreeNode(node_payload));
    }
    return out;
  }

  std::shared_ptr<CoreLibrary> library_;
  TtCoreRuntimeHandle *runtime_handle_ = nullptr;
};

} // namespace

auto MakeCoreApiProxy(std::shared_ptr<CoreLibrary> library,
                      TtCoreRuntimeHandle *runtime_handle)
    -> std::shared_ptr<ITracerCoreApi> {
  return std::make_shared<CoreApiProxy>(std::move(library), runtime_handle);
}

} // namespace tracer_core::cli::bootstrap::internal
