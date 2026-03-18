from __future__ import annotations

from pathlib import Path
from urllib.parse import unquote, urlparse


def resolve_artifact_uri(run: dict, artifact_location: dict | None) -> str:
    if not isinstance(artifact_location, dict):
        return ""
    uri_text = str(artifact_location.get("uri") or "").strip()
    if uri_text:
        return uri_text
    artifact_index = artifact_location.get("index")
    if not isinstance(artifact_index, int):
        return ""
    artifacts = run.get("artifacts")
    if not isinstance(artifacts, list) or artifact_index < 0 or artifact_index >= len(artifacts):
        return ""
    artifact = artifacts[artifact_index]
    if not isinstance(artifact, dict):
        return ""
    nested_location = artifact.get("location")
    if not isinstance(nested_location, dict):
        return ""
    return str(nested_location.get("uri") or "").strip()


def uri_to_path(uri_text: str) -> str:
    normalized_uri = str(uri_text or "").strip()
    if not normalized_uri:
        return ""
    parsed = urlparse(normalized_uri)
    if parsed.scheme != "file":
        return normalized_uri
    path_text = unquote(parsed.path or "")
    if len(path_text) >= 3 and path_text[0] == "/" and path_text[2] == ":":
        path_text = path_text[1:]
    return str(Path(path_text))


def location_to_dict(run: dict, location: dict | None) -> dict:
    if not isinstance(location, dict):
        return {
            "file": "",
            "line": 0,
            "col": 0,
            "end_line": 0,
            "end_col": 0,
            "message": "",
        }
    message_obj = location.get("message")
    physical = location.get("physicalLocation")
    artifact = physical.get("artifactLocation") if isinstance(physical, dict) else None
    region = physical.get("region") if isinstance(physical, dict) else None
    uri_text = resolve_artifact_uri(run, artifact)
    return {
        "file": uri_to_path(uri_text),
        "line": _read_int(region, "startLine"),
        "col": _read_int(region, "startColumn"),
        "end_line": _read_int(region, "endLine"),
        "end_col": _read_int(region, "endColumn"),
        "message": _read_message(message_obj),
    }


def extract_primary_location(run: dict, result: dict) -> dict:
    locations = result.get("locations")
    if isinstance(locations, list) and locations:
        physical = locations[0].get("physicalLocation") if isinstance(locations[0], dict) else None
        location = location_to_dict(run, {"physicalLocation": physical})
        result_message = read_result_message(result)
        if result_message and not location["message"]:
            location["message"] = result_message
        return location

    for event in extract_events(run, result):
        if event.get("file"):
            return {
                "file": str(event.get("file") or ""),
                "line": int(event.get("line") or 0),
                "col": int(event.get("col") or 0),
                "end_line": int(event.get("end_line") or 0),
                "end_col": int(event.get("end_col") or 0),
                "message": str(event.get("message") or ""),
            }

    return {
        "file": "",
        "line": 0,
        "col": 0,
        "end_line": 0,
        "end_col": 0,
        "message": read_result_message(result),
    }


def extract_events(run: dict, result: dict) -> list[dict]:
    events: list[dict] = []
    code_flows = result.get("codeFlows")
    if not isinstance(code_flows, list):
        return events
    for code_flow in code_flows:
        if not isinstance(code_flow, dict):
            continue
        thread_flows = code_flow.get("threadFlows")
        if not isinstance(thread_flows, list):
            continue
        for thread_flow in thread_flows:
            if not isinstance(thread_flow, dict):
                continue
            locations = thread_flow.get("locations")
            if not isinstance(locations, list):
                continue
            for location in locations:
                if not isinstance(location, dict):
                    continue
                issue_location = location_to_dict(run, location.get("location"))
                if not any(issue_location.values()):
                    continue
                events.append(issue_location)
    return events


def read_result_message(result: dict) -> str:
    return _read_message(result.get("message"))


def infer_category(result: dict, rule_id: str) -> str:
    properties = result.get("properties")
    if isinstance(properties, dict):
        category = str(properties.get("category") or "").strip()
        if category:
            return category
    if "." in rule_id:
        return rule_id.split(".", 1)[0]
    return rule_id or "unknown"


def _read_message(message_obj: object) -> str:
    if isinstance(message_obj, dict):
        text = str(message_obj.get("text") or "").strip()
        if text:
            return text
        markdown = str(message_obj.get("markdown") or "").strip()
        if markdown:
            return markdown
    if isinstance(message_obj, str):
        return message_obj.strip()
    return ""


def _read_int(region: object, key: str) -> int:
    if not isinstance(region, dict):
        return 0
    value = region.get(key)
    return value if isinstance(value, int) else 0
