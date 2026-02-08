# Compiler options

option(ENABLE_OPTIMIZATION "Enable compiler optimization flags" ON)
option(ENABLE_PCH "Enable precompiled headers" ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  if(ENABLE_OPTIMIZATION)
    target_compile_options(log_generator PRIVATE
      $<$<CONFIG:Release>:-O3>
      $<$<CONFIG:Release>:-march=native>
    )
  endif()

  target_compile_options(log_generator PRIVATE
    -Wall -Wextra
  )
endif()

# Link-time optimization (Release)
set_property(TARGET log_generator PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
set_property(TARGET log_generator PROPERTY LINK_FLAGS_RELEASE "-s")

# Precompiled headers
if(ENABLE_PCH)
  target_precompile_headers(log_generator PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/pch.hpp")
  message(STATUS "Precompiled Headers enabled: src/pch.hpp")
else()
  message(STATUS "Precompiled Headers disabled.")
endif()
