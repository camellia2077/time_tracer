# Compiler options

option(ENABLE_OPTIMIZATION "Enable compiler optimization flags" ON)
option(DISABLE_OPTIMIZATION "Disable all compiler optimization flags" OFF)
option(ENABLE_LTO "Enable link-time optimization in Release" ON)
option(ENABLE_PCH "Enable precompiled headers" ON)
set(
  WARNING_LEVEL
  "2"
  CACHE STRING
  "Warning level: 0=off, 1=Wall, 2=Wall+Wextra+Wpedantic"
)
set_property(CACHE WARNING_LEVEL PROPERTY STRINGS 0 1 2)

if(DISABLE_OPTIMIZATION)
  set(ENABLE_OPTIMIZATION OFF)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  if(ENABLE_OPTIMIZATION)
    target_compile_options(log_generator PRIVATE
      $<$<CONFIG:Release>:-O3>
      $<$<CONFIG:Release>:-march=native>
    )
  else()
    target_compile_options(log_generator PRIVATE
      $<$<CONFIG:Release>:-O0>
      $<$<CONFIG:RelWithDebInfo>:-O0>
      $<$<CONFIG:MinSizeRel>:-O0>
    )
  endif()

  if(WARNING_LEVEL STREQUAL "0")
    message(STATUS "Compiler warnings disabled (WARNING_LEVEL=0).")
  elseif(WARNING_LEVEL STREQUAL "1")
    target_compile_options(log_generator PRIVATE -Wall)
  elseif(WARNING_LEVEL STREQUAL "2")
    target_compile_options(log_generator PRIVATE -Wall -Wextra -Wpedantic)
  else()
    message(
      WARNING
      "Unknown WARNING_LEVEL='${WARNING_LEVEL}', fallback to 2."
    )
    target_compile_options(log_generator PRIVATE -Wall -Wextra -Wpedantic)
  endif()
endif()

# Link-time optimization (Release)
if(ENABLE_LTO AND ENABLE_OPTIMIZATION)
  set_property(
    TARGET log_generator
    PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
  )
  set_property(TARGET log_generator PROPERTY LINK_FLAGS_RELEASE "-s")
else()
  set_property(
    TARGET log_generator
    PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE FALSE
  )
  set_property(TARGET log_generator PROPERTY LINK_FLAGS_RELEASE "")
endif()

# Precompiled headers
if(ENABLE_PCH)
  target_precompile_headers(log_generator PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/pch.hpp")
  message(STATUS "Precompiled Headers enabled: src/pch.hpp")
else()
  message(STATUS "Precompiled Headers disabled.")
endif()
