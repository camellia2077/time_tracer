set(TRACER_CORE_LIB_SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(TRACER_CORE_LIB_CMAKE_SOURCES_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/sources")

# Layered core aggregation order:
# shared -> domain -> application -> infrastructure(lite)
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/shared.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/domain.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/application.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/infrastructure.cmake")
