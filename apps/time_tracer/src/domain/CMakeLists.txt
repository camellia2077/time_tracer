# src/domain/CMakeLists.txt
add_library(time_tracker_domain STATIC
    "utils/time_utils.cpp"
    "ports/diagnostics.cpp"
    "../shared/utils/period_utils.cpp"
    "logic/converter/convert/core/converter_core.cpp"
    "logic/validator/common/validator_utils.cpp"
    "logic/validator/json/rules/json_rules.cpp"
    "logic/validator/txt/facade/text_validator.cpp"
    "logic/validator/txt/rules/txt_rules.cpp"
    "logic/validator/structure/structure_validator.cpp"
)

setup_plugin_target(time_tracker_domain NO_STDCXXEXP)
