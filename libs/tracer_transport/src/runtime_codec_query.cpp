#include "tracer/transport/runtime_codec.hpp"

#include <stdexcept>
#include <string>

#include "nlohmann/json.hpp"

import tracer.transport.fields;

namespace tracer::transport {

namespace {

using nlohmann::json;
using tracer::transport::modfields::RequireStringField;
using tracer::transport::modfields::TryReadBoolField;
using tracer::transport::modfields::TryReadIntField;
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

auto DecodeQueryRequest(std::string_view request_json) -> QueryRequestPayload {
  const json kPayload = ParseRequestObject(request_json);

  const auto kAction = RequireStringField(kPayload, "action");
  if (kAction.HasError()) {
    throw std::invalid_argument(kAction.error.message);
  }

  QueryRequestPayload out{};
  out.action = kAction.value.value_or("");
  const auto kOutputMode = TryReadStringField(kPayload, "output_mode");

  const auto kYear = TryReadIntField(kPayload, "year");
  const auto kMonth = TryReadIntField(kPayload, "month");
  const auto kFromDate = TryReadStringField(kPayload, "from_date");
  const auto kToDate = TryReadStringField(kPayload, "to_date");
  const auto kRemark = TryReadStringField(kPayload, "remark");
  const auto kDayRemark = TryReadStringField(kPayload, "day_remark");
  const auto kProject = TryReadStringField(kPayload, "project");
  const auto kRoot = TryReadStringField(kPayload, "root");
  const auto kExercise = TryReadIntField(kPayload, "exercise");
  const auto kStatus = TryReadIntField(kPayload, "status");
  const auto kOvernight = TryReadBoolField(kPayload, "overnight");
  const auto kReverse = TryReadBoolField(kPayload, "reverse");
  const auto kLimit = TryReadIntField(kPayload, "limit");
  const auto kTopN = TryReadIntField(kPayload, "top_n");
  const auto kLookbackDays = TryReadIntField(kPayload, "lookback_days");
  const auto kActivityPrefix = TryReadStringField(kPayload, "activity_prefix");
  const auto kScoreByDuration =
      TryReadBoolField(kPayload, "activity_score_by_duration");
  const auto kTreePeriod = TryReadStringField(kPayload, "tree_period");
  const auto kTreePeriodArgument =
      TryReadStringField(kPayload, "tree_period_argument");
  const auto kTreeMaxDepth = TryReadIntField(kPayload, "tree_max_depth");

  if (kYear.HasError()) {
    throw std::invalid_argument(kYear.error.message);
  }
  if (kOutputMode.HasError()) {
    throw std::invalid_argument(kOutputMode.error.message);
  }
  if (kMonth.HasError()) {
    throw std::invalid_argument(kMonth.error.message);
  }
  if (kFromDate.HasError()) {
    throw std::invalid_argument(kFromDate.error.message);
  }
  if (kToDate.HasError()) {
    throw std::invalid_argument(kToDate.error.message);
  }
  if (kRemark.HasError()) {
    throw std::invalid_argument(kRemark.error.message);
  }
  if (kDayRemark.HasError()) {
    throw std::invalid_argument(kDayRemark.error.message);
  }
  if (kProject.HasError()) {
    throw std::invalid_argument(kProject.error.message);
  }
  if (kRoot.HasError()) {
    throw std::invalid_argument(kRoot.error.message);
  }
  if (kExercise.HasError()) {
    throw std::invalid_argument(kExercise.error.message);
  }
  if (kStatus.HasError()) {
    throw std::invalid_argument(kStatus.error.message);
  }
  if (kOvernight.HasError()) {
    throw std::invalid_argument(kOvernight.error.message);
  }
  if (kReverse.HasError()) {
    throw std::invalid_argument(kReverse.error.message);
  }
  if (kLimit.HasError()) {
    throw std::invalid_argument(kLimit.error.message);
  }
  if (kTopN.HasError()) {
    throw std::invalid_argument(kTopN.error.message);
  }
  if (kLookbackDays.HasError()) {
    throw std::invalid_argument(kLookbackDays.error.message);
  }
  if (kActivityPrefix.HasError()) {
    throw std::invalid_argument(kActivityPrefix.error.message);
  }
  if (kScoreByDuration.HasError()) {
    throw std::invalid_argument(kScoreByDuration.error.message);
  }
  if (kTreePeriod.HasError()) {
    throw std::invalid_argument(kTreePeriod.error.message);
  }
  if (kTreePeriodArgument.HasError()) {
    throw std::invalid_argument(kTreePeriodArgument.error.message);
  }
  if (kTreeMaxDepth.HasError()) {
    throw std::invalid_argument(kTreeMaxDepth.error.message);
  }

  out.output_mode = kOutputMode.value;
  out.year = kYear.value;
  out.month = kMonth.value;
  out.from_date = kFromDate.value;
  out.to_date = kToDate.value;
  out.remark = kRemark.value;
  out.day_remark = kDayRemark.value;
  out.project = kProject.value;
  out.root = kRoot.value;
  out.exercise = kExercise.value;
  out.status = kStatus.value;
  out.overnight = kOvernight.value;
  out.reverse = kReverse.value;
  out.limit = kLimit.value;
  out.top_n = kTopN.value;
  out.lookback_days = kLookbackDays.value;
  out.activity_prefix = kActivityPrefix.value;
  out.activity_score_by_duration = kScoreByDuration.value;
  out.tree_period = kTreePeriod.value;
  out.tree_period_argument = kTreePeriodArgument.value;
  out.tree_max_depth = kTreeMaxDepth.value;

  return out;
}

auto EncodeQueryRequest(const QueryRequestPayload& request) -> std::string {
  json payload = {
      {"action", request.action},
  };
  if (request.output_mode.has_value()) {
    payload["output_mode"] = *request.output_mode;
  }
  if (request.year.has_value()) {
    payload["year"] = *request.year;
  }
  if (request.month.has_value()) {
    payload["month"] = *request.month;
  }
  if (request.from_date.has_value()) {
    payload["from_date"] = *request.from_date;
  }
  if (request.to_date.has_value()) {
    payload["to_date"] = *request.to_date;
  }
  if (request.remark.has_value()) {
    payload["remark"] = *request.remark;
  }
  if (request.day_remark.has_value()) {
    payload["day_remark"] = *request.day_remark;
  }
  if (request.project.has_value()) {
    payload["project"] = *request.project;
  }
  if (request.root.has_value()) {
    payload["root"] = *request.root;
  }
  if (request.exercise.has_value()) {
    payload["exercise"] = *request.exercise;
  }
  if (request.status.has_value()) {
    payload["status"] = *request.status;
  }
  if (request.overnight.has_value()) {
    payload["overnight"] = *request.overnight;
  }
  if (request.reverse.has_value()) {
    payload["reverse"] = *request.reverse;
  }
  if (request.limit.has_value()) {
    payload["limit"] = *request.limit;
  }
  if (request.top_n.has_value()) {
    payload["top_n"] = *request.top_n;
  }
  if (request.lookback_days.has_value()) {
    payload["lookback_days"] = *request.lookback_days;
  }
  if (request.activity_prefix.has_value()) {
    payload["activity_prefix"] = *request.activity_prefix;
  }
  if (request.activity_score_by_duration.has_value()) {
    payload["activity_score_by_duration"] = *request.activity_score_by_duration;
  }
  if (request.tree_period.has_value()) {
    payload["tree_period"] = *request.tree_period;
  }
  if (request.tree_period_argument.has_value()) {
    payload["tree_period_argument"] = *request.tree_period_argument;
  }
  if (request.tree_max_depth.has_value()) {
    payload["tree_max_depth"] = *request.tree_max_depth;
  }
  return payload.dump();
}

auto EncodeQueryResponse(const QueryResponsePayload& response) -> std::string {
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
