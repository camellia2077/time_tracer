#include "tracer/transport/runtime_codec.hpp"

#include <stdexcept>
#include <string>

#include "nlohmann/json.hpp"
#include "tracer/transport/fields.hpp"

namespace tracer::transport {

namespace {

using nlohmann::json;

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
  const json payload = ParseRequestObject(request_json);

  const auto action = RequireStringField(payload, "action");
  if (action.HasError()) {
    throw std::invalid_argument(action.error.message);
  }

  QueryRequestPayload out{};
  out.action = *action.value;
  const auto output_mode = TryReadStringField(payload, "output_mode");

  const auto year = TryReadIntField(payload, "year");
  const auto month = TryReadIntField(payload, "month");
  const auto from_date = TryReadStringField(payload, "from_date");
  const auto to_date = TryReadStringField(payload, "to_date");
  const auto remark = TryReadStringField(payload, "remark");
  const auto day_remark = TryReadStringField(payload, "day_remark");
  const auto project = TryReadStringField(payload, "project");
  const auto root = TryReadStringField(payload, "root");
  const auto exercise = TryReadIntField(payload, "exercise");
  const auto status = TryReadIntField(payload, "status");
  const auto overnight = TryReadBoolField(payload, "overnight");
  const auto reverse = TryReadBoolField(payload, "reverse");
  const auto limit = TryReadIntField(payload, "limit");
  const auto top_n = TryReadIntField(payload, "top_n");
  const auto lookback_days = TryReadIntField(payload, "lookback_days");
  const auto activity_prefix = TryReadStringField(payload, "activity_prefix");
  const auto score_by_duration =
      TryReadBoolField(payload, "activity_score_by_duration");
  const auto tree_period = TryReadStringField(payload, "tree_period");
  const auto tree_period_argument =
      TryReadStringField(payload, "tree_period_argument");
  const auto tree_max_depth = TryReadIntField(payload, "tree_max_depth");

  if (year.HasError()) {
    throw std::invalid_argument(year.error.message);
  }
  if (output_mode.HasError()) {
    throw std::invalid_argument(output_mode.error.message);
  }
  if (month.HasError()) {
    throw std::invalid_argument(month.error.message);
  }
  if (from_date.HasError()) {
    throw std::invalid_argument(from_date.error.message);
  }
  if (to_date.HasError()) {
    throw std::invalid_argument(to_date.error.message);
  }
  if (remark.HasError()) {
    throw std::invalid_argument(remark.error.message);
  }
  if (day_remark.HasError()) {
    throw std::invalid_argument(day_remark.error.message);
  }
  if (project.HasError()) {
    throw std::invalid_argument(project.error.message);
  }
  if (root.HasError()) {
    throw std::invalid_argument(root.error.message);
  }
  if (exercise.HasError()) {
    throw std::invalid_argument(exercise.error.message);
  }
  if (status.HasError()) {
    throw std::invalid_argument(status.error.message);
  }
  if (overnight.HasError()) {
    throw std::invalid_argument(overnight.error.message);
  }
  if (reverse.HasError()) {
    throw std::invalid_argument(reverse.error.message);
  }
  if (limit.HasError()) {
    throw std::invalid_argument(limit.error.message);
  }
  if (top_n.HasError()) {
    throw std::invalid_argument(top_n.error.message);
  }
  if (lookback_days.HasError()) {
    throw std::invalid_argument(lookback_days.error.message);
  }
  if (activity_prefix.HasError()) {
    throw std::invalid_argument(activity_prefix.error.message);
  }
  if (score_by_duration.HasError()) {
    throw std::invalid_argument(score_by_duration.error.message);
  }
  if (tree_period.HasError()) {
    throw std::invalid_argument(tree_period.error.message);
  }
  if (tree_period_argument.HasError()) {
    throw std::invalid_argument(tree_period_argument.error.message);
  }
  if (tree_max_depth.HasError()) {
    throw std::invalid_argument(tree_max_depth.error.message);
  }

  out.output_mode = output_mode.value;
  out.year = year.value;
  out.month = month.value;
  out.from_date = from_date.value;
  out.to_date = to_date.value;
  out.remark = remark.value;
  out.day_remark = day_remark.value;
  out.project = project.value;
  out.root = root.value;
  out.exercise = exercise.value;
  out.status = status.value;
  out.overnight = overnight.value;
  out.reverse = reverse.value;
  out.limit = limit.value;
  out.top_n = top_n.value;
  out.lookback_days = lookback_days.value;
  out.activity_prefix = activity_prefix.value;
  out.activity_score_by_duration = score_by_duration.value;
  out.tree_period = tree_period.value;
  out.tree_period_argument = tree_period_argument.value;
  out.tree_max_depth = tree_max_depth.value;

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

auto EncodeQueryResponse(const QueryResponsePayload& response)
    -> std::string {
  return json{
      {"ok", response.ok},
      {"content", response.content},
      {"error_message", response.error_message},
  }
      .dump();
}

}  // namespace tracer::transport
