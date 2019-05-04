cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ---- Options ----

option(ENABLE_TEST_COVERAGE "Enable test coverage" OFF)

# ---- Dependencies ----

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  find_package(LarsParser REQUIRED)
endif()

include(../cmake/CPM.cmake)

CPMAddPackage(
  NAME Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  VERSION 2.5.0
)

# ---- Create binary ----

file(GLOB tests_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(LarsParserTests ${tests_sources})
target_link_libraries(LarsParserTests PUBLIC LarsParser Catch2)
set_target_properties(LarsParserTests PROPERTIES CXX_STANDARD 17 COMPILE_FLAGS "-Wall -pedantic -Wextra -Werror")

# ---- Add tests ----

ENABLE_TESTING() 
ADD_TEST(LarsParserTests LarsParserTests)

# ---- code coverage ----

if (${ENABLE_TEST_COVERAGE})
  set_target_properties(LarsParser PROPERTIES CXX_STANDARD 17 COMPILE_FLAGS "-O0 -g -fprofile-arcs -ftest-coverage --coverage")
  target_link_options(LarsParser PUBLIC "--coverage")
  set_target_properties(LarsParserTests PROPERTIES CXX_STANDARD 17 COMPILE_FLAGS "-O0 -g -fprofile-arcs -ftest-coverage --coverage")
  target_link_options(LarsParserTests PUBLIC "--coverage")
endif()
