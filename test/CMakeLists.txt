cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(PEGParserTests LANGUAGES CXX)

# ---- Options ----

option(ENABLE_TEST_COVERAGE "Enable test coverage" OFF)
option(TEST_INSTALLED_VERSION "Test the version found by find_package" OFF)

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

CPMAddPackage(
  NAME Catch2
  GITHUB_REPOSITORY catchorg/Catch2
  VERSION 2.13.4
)

if(TEST_INSTALLED_VERSION)
  find_package(PEGParser REQUIRED)
  find_package(PEGParserGlue REQUIRED)
else()
  CPMAddPackage(NAME PEGParser SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
  CPMAddPackage(NAME PEGParserGlue SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../glue)
endif()

CPMAddPackage(
  NAME Format.cmake
  GITHUB_REPOSITORY TheLartians/Format.cmake
  VERSION 1.6
  OPTIONS "FORMAT_CHECK_CMAKE ON"
)

# ---- Create binary ----

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
add_executable(PEGParserTests ${sources})
target_link_libraries(PEGParserTests Catch2 PEGParser::PEGParser PEGParserGlue::PEGParserGlue)

set_target_properties(PEGParserTests PROPERTIES CXX_STANDARD 17)

# enable compiler warnings
if(NOT TEST_INSTALLED_VERSION)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(PEGParser PUBLIC -Wall -pedantic -Wextra -Werror)
  elseif(MSVC)
    target_compile_options(PEGParser PUBLIC /W4 /WX /wd4456)
    target_compile_definitions(PEGParserTests PUBLIC DOCTEST_CONFIG_USE_STD_HEADERS)
  endif()
endif()

# ---- Add PEGParserTests ----

enable_testing()

include(${Catch2_SOURCE_DIR}/contrib/Catch.cmake)
catch_discover_tests(PEGParserTests)

# ---- code coverage ----

if(ENABLE_TEST_COVERAGE)
  target_compile_options(PEGParser PUBLIC -O0 -g -fprofile-arcs -ftest-coverage)
  target_link_options(PEGParser PUBLIC -fprofile-arcs -ftest-coverage)
endif()
