if(TARGET tc_core_bridge_common_c_api_lib)
    return()
endif()

add_library(tc_core_bridge_common_c_api_lib STATIC
    "src/jni/bridge_utils.cpp"
)

target_include_directories(tc_core_bridge_common_c_api_lib PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/src"
    "${CMAKE_CURRENT_LIST_DIR}/../tracer_core/src"
    "${CMAKE_CURRENT_LIST_DIR}/../tracer_transport/include"
)

target_link_libraries(tc_core_bridge_common_c_api_lib PUBLIC
    nlohmann_json::nlohmann_json
)
