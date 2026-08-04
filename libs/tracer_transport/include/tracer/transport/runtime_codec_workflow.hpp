#pragma once

#include <string>
#include <string_view>

#include "tracer/transport/runtime_requests.hpp"

namespace tracer::transport {

[[nodiscard]] auto DecodeConvertRequest(std::string_view request_json)
    -> ConvertRequestPayload;
[[nodiscard]] auto EncodeConvertRequest(const ConvertRequestPayload& request)
    -> std::string;

[[nodiscard]] auto DecodeImportRequest(std::string_view request_json)
    -> ImportRequestPayload;
[[nodiscard]] auto EncodeImportRequest(const ImportRequestPayload& request)
    -> std::string;

[[nodiscard]] auto DecodeValidateStructureRequest(std::string_view request_json)
    -> ValidateStructureRequestPayload;
[[nodiscard]] auto EncodeValidateStructureRequest(
    const ValidateStructureRequestPayload& request) -> std::string;

[[nodiscard]] auto DecodeValidateLogicRequest(std::string_view request_json)
    -> ValidateLogicRequestPayload;
[[nodiscard]] auto EncodeValidateLogicRequest(
    const ValidateLogicRequestPayload& request) -> std::string;

[[nodiscard]] auto DecodeRecordActivityAtomicallyRequest(
    std::string_view request_json) -> RecordActivityAtomicallyRequestPayload;
[[nodiscard]] auto EncodeRecordActivityAtomicallyRequest(
    const RecordActivityAtomicallyRequestPayload& request) -> std::string;

[[nodiscard]] auto DecodeUpdateActivityRemarkAtomicallyRequest(
    std::string_view request_json)
    -> UpdateActivityRemarkAtomicallyRequestPayload;
[[nodiscard]] auto EncodeUpdateActivityRemarkAtomicallyRequest(
    const UpdateActivityRemarkAtomicallyRequestPayload& request) -> std::string;

[[nodiscard]] auto DecodeUpdateDayRemarkAtomicallyRequest(
    std::string_view request_json) -> UpdateDayRemarkAtomicallyRequestPayload;
[[nodiscard]] auto EncodeUpdateDayRemarkAtomicallyRequest(
    const UpdateDayRemarkAtomicallyRequestPayload& request) -> std::string;

}  // namespace tracer::transport
