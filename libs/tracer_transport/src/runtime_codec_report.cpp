#include "tracer/transport/runtime_codec.hpp"

#include <stdexcept>
#include <string>

#include "nlohmann/json.hpp"

import tracer.transport.fields;

namespace tracer::transport {

namespace {

using nlohmann::json;
using tracer::transport::modfields::RequireStringField;
using tracer::transport::modfields::TryReadIntField;
using tracer::transport::modfields::TryReadIntListField;
using tracer::transport::modfields::TryReadStringField;

auto ParseRequestObject(std::string_view request_json) -> json {
  if (request_json.empty()) {
    throw std::invalid_argument("request_json must not be empty.");
  }
  json payload = json::parse(std::string(request_json));
  if (!payload.is_object()) {
    throw std::invalid_argument("request_json must be a JSON object.");
  }
  return payload;
}

}  // namespace

auto EncodeReportResponse(const ReportResponsePayload& response)
    -> std::string {
  return json{
      {"ok", response.ok},
      {"content", response.content},
      {"error_message", response.error_message},
      {"error_code", response.error_contract.error_code},
      {"error_category", response.error_contract.error_category},
      {"hints", response.error_contract.hints},
  }
      .dump();
}

auto DecodeTemporalReportRequest(std::string_view request_json)
    -> TemporalReportRequestPayload {
  const json kPayload = ParseRequestObject(request_json);

  const auto kOperationKind = RequireStringField(kPayload, "operation_kind");
  const auto kDisplayMode = RequireStringField(kPayload, "display_mode");
  const auto kSelectionKind = TryReadStringField(kPayload, "selection_kind");
  const auto kDate = TryReadStringField(kPayload, "date");
  const auto kStartDate = TryReadStringField(kPayload, "start_date");
  const auto kEndDate = TryReadStringField(kPayload, "end_date");
  const auto kDays = TryReadIntField(kPayload, "days");
  const auto kAnchorDate = TryReadStringField(kPayload, "anchor_date");
  const auto kFormat = TryReadStringField(kPayload, "format");
  const auto kExportScope = TryReadStringField(kPayload, "export_scope");
  const auto kRecentDays = TryReadIntListField(kPayload, "recent_days_list");
  if (kOperationKind.HasError()) {
    throw std::invalid_argument(kOperationKind.error.message);
  }
  if (kDisplayMode.HasError()) {
    throw std::invalid_argument(kDisplayMode.error.message);
  }
  if (kSelectionKind.HasError()) {
    throw std::invalid_argument(kSelectionKind.error.message);
  }
  if (kDate.HasError()) {
    throw std::invalid_argument(kDate.error.message);
  }
  if (kStartDate.HasError()) {
    throw std::invalid_argument(kStartDate.error.message);
  }
  if (kEndDate.HasError()) {
    throw std::invalid_argument(kEndDate.error.message);
  }
  if (kDays.HasError()) {
    throw std::invalid_argument(kDays.error.message);
  }
  if (kAnchorDate.HasError()) {
    throw std::invalid_argument(kAnchorDate.error.message);
  }
  if (kFormat.HasError()) {
    throw std::invalid_argument(kFormat.error.message);
  }
  if (kExportScope.HasError()) {
    throw std::invalid_argument(kExportScope.error.message);
  }
  if (kRecentDays.HasError()) {
    throw std::invalid_argument(kRecentDays.error.message);
  }

  TemporalReportRequestPayload out{};
  out.operation_kind = kOperationKind.value.value_or("");
  out.display_mode = kDisplayMode.value.value_or("");
  out.selection_kind = kSelectionKind.value;
  out.date = kDate.value;
  out.start_date = kStartDate.value;
  out.end_date = kEndDate.value;
  out.days = kDays.value;
  out.anchor_date = kAnchorDate.value;
  out.format = kFormat.value;
  out.export_scope = kExportScope.value;
  out.recent_days_list = kRecentDays.value;
  return out;
}

auto EncodeTemporalReportRequest(const TemporalReportRequestPayload& request)
    -> std::string {
  json payload = {
      {"operation_kind", request.operation_kind},
      {"display_mode", request.display_mode},
  };
  if (request.selection_kind.has_value()) {
    payload["selection_kind"] = *request.selection_kind;
  }
  if (request.date.has_value()) {
    payload["date"] = *request.date;
  }
  if (request.start_date.has_value()) {
    payload["start_date"] = *request.start_date;
  }
  if (request.end_date.has_value()) {
    payload["end_date"] = *request.end_date;
  }
  if (request.days.has_value()) {
    payload["days"] = *request.days;
  }
  if (request.anchor_date.has_value()) {
    payload["anchor_date"] = *request.anchor_date;
  }
  if (request.format.has_value()) {
    payload["format"] = *request.format;
  }
  if (request.export_scope.has_value()) {
    payload["export_scope"] = *request.export_scope;
  }
  if (request.recent_days_list.has_value()) {
    payload["recent_days_list"] = *request.recent_days_list;
  }
  return payload.dump();
}

auto EncodeReportTargetsResponse(const ReportTargetsResponsePayload& response)
    -> std::string {
  return json{
      {"ok", response.ok},
      {"type", response.type},
      {"items", response.items},
      {"error_message", response.error_message},
      {"error_code", response.error_contract.error_code},
      {"error_category", response.error_contract.error_category},
      {"hints", response.error_contract.hints},
  }
      .dump();
}

auto DecodeReportBatchRequest(std::string_view request_json)
    -> ReportBatchRequestPayload {
  const json kPayload = ParseRequestObject(request_json);

  const auto kDaysList = TryReadIntListField(kPayload, "days_list");
  if (kDaysList.HasError()) {
    throw std::invalid_argument(kDaysList.error.message);
  }
  if (!kDaysList.value.has_value()) {
    throw std::invalid_argument("field `days_list` must be an integer array.");
  }

  const auto kFormat = TryReadStringField(kPayload, "format");
  if (kFormat.HasError()) {
    throw std::invalid_argument(kFormat.error.message);
  }

  ReportBatchRequestPayload out{};
  out.days_list = *kDaysList.value;
  out.format = kFormat.value;
  return out;
}

auto EncodeReportBatchRequest(const ReportBatchRequestPayload& request)
    -> std::string {
  json payload = {
      {"days_list", request.days_list},
  };
  if (request.format.has_value()) {
    payload["format"] = *request.format;
  }
  return payload.dump();
}

auto EncodeReportBatchResponse(const ReportBatchResponsePayload& response)
    -> std::string {
  return json{
      {"ok", response.ok},
      {"content", response.content},
      {"error_message", response.error_message},
      {"error_code", response.error_contract.error_code},
      {"error_category", response.error_contract.error_category},
      {"hints", response.error_contract.hints},
  }
      .dump();
}

}  // namespace tracer::transport
