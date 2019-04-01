cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

# ---- Project ----
project(LarsParser CXX)

if(${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif()

# ---- Configuration variables ----
set(LARS_PARSER_EXAMPLES FALSE CACHE BOOL "build the Lars::Parser examples")

# ---- Dependencies ----
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/get_lhc.cmake)

# ---- Minimum CXX ----
if(NOT CMAKE_CXX_STANDARD GREATER 14)
  set(CMAKE_CXX_STANDARD 14)
endif()

# ---- Create library ----
FILE(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/include/lars/*.h")
add_library(LarsParser EXCLUDE_FROM_ALL ${headers})
SET_TARGET_PROPERTIES(LarsParser PROPERTIES LINKER_LANGUAGE CXX)
add_dependencies(LarsParser lhc-project)

# ---- Export ----
set(LARS_PARSER_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include/" ${LHC_INCLUDE_DIRS} CACHE INTERNAL "The Lars::Parser include dirs")

# ---- Examples ----
IF(LARS_PARSER_EXAMPLES)
  file( GLOB example_sources ${CMAKE_CURRENT_SOURCE_DIR}/examples/*.cpp )
  foreach( example_source_file ${example_sources} )
    get_filename_component(filename ${example_source_file} NAME)
    string(REPLACE ".cpp" "" example_name ${filename} )
    add_executable(${example_name} ${example_source_file})
    include_directories(${LARS_PARSER_INCLUDE_DIRS})
  endforeach()
ENDIF()

