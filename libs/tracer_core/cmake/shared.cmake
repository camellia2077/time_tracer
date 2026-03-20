add_library(tc_shared_lib STATIC
    "${TRACER_CORE_LIB_SOURCE_ROOT}/shared/utils/period_utils.cpp"
)

target_sources(tc_shared_lib PUBLIC
    FILE_SET core_shared_modules TYPE CXX_MODULES
    BASE_DIRS
        "${TRACER_CORE_LIB_SOURCE_ROOT}/shared/modules"
    FILES
        "${TRACER_CORE_LIB_SOURCE_ROOT}/shared/modules/tracer.core.shared.canonical_text.cppm"
        "${TRACER_CORE_LIB_SOURCE_ROOT}/shared/modules/tracer.core.shared.string_utils.cppm"
        "${TRACER_CORE_LIB_SOURCE_ROOT}/shared/modules/tracer.core.shared.period_utils.cppm"
        "${TRACER_CORE_LIB_SOURCE_ROOT}/shared/modules/tracer.core.shared.exceptions.cppm"
        "${TRACER_CORE_LIB_SOURCE_ROOT}/shared/modules/tracer.core.shared.exit_codes.cppm"
        "${TRACER_CORE_LIB_SOURCE_ROOT}/shared/modules/tracer.core.shared.cppm"
)
set_target_properties(tc_shared_lib PROPERTIES
    CXX_SCAN_FOR_MODULES ON
)

if(COMMAND setup_plugin_target)
    setup_plugin_target(tc_shared_lib NO_STDCXXEXP NO_PCH NO_APP_SOURCE_ROOT_INCLUDE)
elseif(COMMAND setup_app_target)
    setup_app_target(tc_shared_lib NO_PCH NO_APP_SOURCE_ROOT_INCLUDE)
endif()

target_include_directories(tc_shared_lib PUBLIC
    "${TRACER_CORE_LIB_SOURCE_ROOT}"
)
